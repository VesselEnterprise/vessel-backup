<?php

require_once 'database.class.php';
require_once 'log.class.php';
require_once 'user.class.php';

class BackupSession
{
	
	private static $factory;
	private $_userId = -1;
	private $_dbconn;
	private $_loginSuccess;
	private $_sessionHash;
	private $_sessionId = -1;
	private $_sessionExpired;
	private $_ip_address;
	private $_log;
	
	public function __construct() {
		
		//Set remote IP
		$this->_ip_address = $_SERVER['REMOTE_ADDR'];
		
		//Get Database Connection
		$this->_dbconn = BackupDatabase::getDatabase()->getConnection();
		
		//Get Log Obj
		$this->_log = BackupLog::getLog( $this->_userId );
		
		//Get or Create User Session		
		$this->_initSession();
		
	}
	
	function __destruct() {
		
	}
	
	public function getIPAddr() {
		return $this->_ip_address;
	}
	
	public static function getSession()
	{
		if (!self::$factory)
			self::$factory = new BackupSession();
		
		return self::$factory;
	}
	
	public function login($username,$pwd) {
		
		//Get User Password Hash
		if ( $stmt = mysqli_prepare($this->_dbconn, "SELECT user_id, password FROM backup_user WHERE user_name=?") ) {
			
			$stmt->bind_param('s', $username);
			$stmt->execute();
			
			$result = $stmt->get_result();
			
			if ( $row = $result->fetch_row() ) {
				
				if ( password_verify($pwd, $row[1]) == TRUE ) {
					
					//Set UserId
					$this->_userId = $row[0];
					
					//Update user Id for the current session
					$this->_updateSessionUserId($this->_userId);
					
					//Create new session
					$this->_initSession();
					
					//Password is correct, login successful
					$this->_updateLastLogin();
					
					$this->_loginSuccess=true;
					
				}
				else {
					$this->_loginSuccess=false;
				}
				
			}
			
			$stmt->close();
			
		}
		
		return $this->_loginSuccess;

	}
	
	public function isLoggedIn() {
		return $this->_loginSuccess;
	}
	
	private function _updateSessionUserId($user_id) {
		
		$query = "UPDATE backup_user_session SET user_id=? WHERE session_hash=?";
		
		if ( $stmt = mysqli_prepare($this->_dbconn,$query) ) {
			
			$stmt->bind_param('is', $user_id, $this->_sessionHash );
			
			if ( $stmt->execute() ) {
				$this->_log->addMessage("Updated session user Id (" . $user_id . ")", "Authentication");
			}
			
			$stmt->close();
			
			setcookie('user_id', $user_id );
			
		}
		
	}
	
	public function logout() {
		
		//Clear Cookies
		$this->_clearCookies();
		
		//Expire Session
		if ( !empty($this->_sessionHash) ) {
			$this->_expireSession();
		}		
		
		session_unset();
		session_destroy();
	}
	
	public function getUserId() {
		return $this->_userId;
	}
	
	private function _updateLastLogin() {
		
		if ( !$this->_userId )
			return;
		
		mysqli_query($this->_dbconn, "UPDATE backup_user SET last_login = NOW() WHERE user_id = '" . $this->_userId . "'");
		
	}
	
	private function _createSessionKey() {
		return bin2hex(random_bytes(32));
	}
	
	public function getSessionId() {
		
		$sessionId = '';
		
		$query = "SELECT session_id FROM backup_user_session WHERE session_hash = '" . $this->_sessionHash . "'";
		if ( $result = mysqli_query($this->_dbconn, $query) ) {
			$sessionId = $result[0];
		}
		
		return $sessionId;		
		
	}
	
	private function _validateSession() {
		
		$isValid=false;
		
		$query = "SELECT session_id,UNIX_TIMESTAMP(last_accessed),user_id FROM backup_user_session WHERE session_hash = '" . $this->_sessionHash . "' AND expired=0";
		if ( $result = mysqli_query($this->_dbconn, $query) ) {
			
			if ( $row = mysqli_fetch_row($result) ) {
				
				$d = new DateTime();
				$d->setTimestamp( $row[1] );
				$d->add( new DateInterval('P1D') );
				
				//Check if session is >= 24 hours old
				if ( time() >= $d->getTimestamp() ) {
					
					$this->_expireSession($this->_sessionHash);
					
					$isValid=false;
				}
				else {
					$isValid=true;
					/* If session hash is valid, not expired, and user Id is > 0, user is logged in */
					if ( $row[2] > 0 )
						$this->_loginSuccess=true; //User is logged in
				}
				
			}
			
			$result->close();
			
		}
		
		return $isValid;
		
	}
	
	private function _expireSession() {
		
		$query = "UPDATE backup_user_session SET expired=1 WHERE session_hash=?";
		if ( $stmt = mysqli_prepare($this->_dbconn, $query) ) {
			
			$stmt->bind_param('s', $this->_sessionHash );
			
			if ( $stmt->execute() ) {
				
				$this->_sessionExpired=true;
				$this->_sessionHash = '';
				$this->_sessionId = '';
				
				$this->_log->addMessage("User session has been expired (" . $this->_sessionHash . ")", "Authentication");
				
			}
			
			$stmt->close();
			
		}
		
	}
	
	private function _hashSessionKey($key) {
		return hash('sha512', $key );
	}
	
	private function _initSession() {
		
		session_start();
		
		//Check if the user has an existing sessionId
		if ( !empty($_COOKIE['session_key'] ) ) {
			
			$this->_sessionHash = $this->_hashSessionKey($_COOKIE['session_key']);
			
			if ( !empty($_COOKIE['user_id'] ) ) {
				$this->_userId = $_COOKIE['user_id'];
				$this->_log->setUserId( $this->_userId );
			}
			
			//Validate session
			if ( $this->_validateSession() ) {
				
				//User is already logged in
				//Update last access time
				
				$this->_updateLastAccessed();
				return;
			}
			else {
				//Clear cookies and force login
				$this->_clearCookies();
			}
			
		}
		
		//Create new session
		if ( !$this->_userId )
			$this->_userId = -1; //User is not logged in
		
		$sessionKey = $this->_createSessionKey();
		$this->_sessionHash = $this->_hashSessionKey($sessionKey);
		$ip = $this->getIPAddr();
		
		$query = "INSERT INTO backup_user_session (user_id,session_hash,ip_address) VALUES(?,?,?)";
		
		if ( $stmt = mysqli_prepare($this->_dbconn, $query ) ) {
			
			$stmt->bind_param(
				"iss",
				$this->_userId,
				$this->_sessionHash,
				$ip
			);
			
			if ( $stmt->execute() ) {
				
				$this->_sessionId = mysqli_insert_id($this->_dbconn);
				$this->_sessionHash = $this->_hashSessionKey($sessionKey);
				
				setcookie("session_key", $sessionKey);
				setcookie("user_id", $this->_userId );
				
			}
			
			$stmt->close();
			
		}
		
	}
	
	private function _clearCookies() {
		setcookie('session_key', '', time() - 3600);
		setcookie('user_id', '', time() - 3600 );
	}
	
	private function _updateLastAccessed() {
		
		@mysqli_query(
			$this->_dbconn, 
			"UPDATE backup_user_session SET last_accessed=NOW(),ip_address='" . $this->getIPAddr() ."' WHERE session_hash='" . $this->_sessionHash . "'"
		);
		
	}
	
	public function printLoginMsg() {
		
		echo "Error: You must be authenticated to use this feature. Please <a href=\"user.php?action=login\">login</a> to continue";
		
	}
	
	public function getRefererURL() {
		return !empty($_SERVER['HTTP_REFERER']) ? $_SERVER['HTTP_REFERER'] : "index.php";
	}
	
}

?>