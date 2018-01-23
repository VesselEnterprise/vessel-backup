<?php

require_once 'database.class.php';

class BackupUser
{
	
	private static $factory;
	
	private $_userId;
	private $_db;
	private $_dbconn;
	private $_log;
	private $_userRow;
	
	public function __construct($userId) {
		
		$this->_userId = $userId;
		
		//Get database connection
		$this->_db = BackupDatabase::getDatabase();
		$this->_dbconn = $this->_db->getConnection();
		
		//Get Log Object
		$this->_log = BackupLog::getLog($userId);
		
		$this->_userRow = $this->_getUserRow($userId);
		
	}
	
	function __destruct() {
		
	}
	
	private function _getUserRow($userId) {
		
		$row = array();
		
		$userName = (string)$userId;
		
		$query = "SELECT * FROM backup_user WHERE user_id=? OR user_name=?";
		if ( $stmt = mysqli_prepare($this->_dbconn, $query ) ) {
			$stmt->bind_param('is', $userId, $userName );
			if ( $stmt->execute() ) {
				$result = $stmt->get_result();
				$row = mysqli_fetch_array($result);
			}
			$stmt->close();
		}
		
		//Check if user_name was provided instead
		if ( gettype($userId) == "string" )
			$this->_userId = $row['user_id'];
		
		return $row;		
		
	}
	
	public function getValue($col) {
		return $this->_userRow[$col];
	}
	
	public static function getUser($userId)
	{
		if (!self::$factory)
			self::$factory = new BackupUser($userId);
		
		return self::$factory;
	}
	
	public function setPassword($pwd) {
		
		$hash = password_hash( $pwd, PASSWORD_DEFAULT );
		
		//Update user password
		if ( $stmt = mysqli_prepare("UPDATE backup_user SET password=?,password_set=NOW() WHERE userId=?") ) {
			
			$stmt->bind_param('si', $hash, $this->_userId );
			$stmt->execute();
			$stmt->close();
			
			$this->_log->addMessage("User (User Id = " . $this->_userId . ") changed their password");
			
		}		
		
	}
	
	public static function generateAccessToken() {
		return substr(bin2hex(random_bytes(32)),0,32);
	}
	
	public function createUserActivation($code='') {
		
		$status=false;
		
		if ( !$code )
			$code = $this->generateAccessToken();
		
		$query = "INSERT INTO backup_user_activation (user_id,code,expiry) VALUES(?,?,(NOW() + INTERVAL 7 DAY))";
		
		if ( $stmt = mysqli_prepare($this->_dbconn, $query ) ) {
			
			$stmt->bind_param('is', $this->_userId, $code );
			if ( $stmt->execute() ) {
				$status=true;
			}
			else
				echo "Error: " . mysqli_error($this->_dbconn);
			
			$stmt->close();
			
		}
		
		return $status;
		
	}
	
	public function activateUser($activationCode) {
		
		$accessToken = '';
		
		$query = "SELECT user_id FROM backup_user_activation WHERE user_id=? AND code=? AND expiry >= NOW()";
		
		if ( $stmt = mysqli_prepare($this->_dbconn, $query ) ) {
			
			$stmt->bind_param('is', $this->_userId, $activationCode );
			
			if ( $stmt->execute() ) {
				
				$result = $stmt->get_result();
				if ( $result->num_rows > 0 ) {
					
					//User is now activated
					$accessToken = $this->generateAccessToken();
					$accessTokenHashed = sha1( $accessToken );
					
					//Set the user's access key
					if ( $s2 = mysqli_prepare($this->_dbconn, "UPDATE backup_user SET access_token=?,token_expiry=(NOW()+INTERVAL ? HOUR) WHERE user_id=?") ) {
						
						//Get token lifetime
						$tokenExpiry = (int)$this->_db->getSetting('token_expiry');
						
						$s2->bind_param('sii', $accessTokenHashed, $tokenExpiry, $this->_userId);
						if ( $s2->execute() ) {
							$this->_log->addMessage("Successfully activated user (user_id=" . $this->_userId . ")", "User Activation");
						}
						$s2->close();
						
						//Remove the user activation record
						@mysqli_query($this->_dbconn, "DELETE FROM backup_user_activation WHERE user_id=" . $this->_userId );
					}
					else {
						$this->_log->addError("Error: Failed to activate user (user_id=" . $this->_userId . ") (" . mysqli_error($this->_dbconn) . ")", "User Activation");
					}				
					
				}
				else {
					$this->_log->addError("Error: Failed to activate user (user_id=" . $this->_userId . ") (" . mysqli_error($this->_dbconn) . ")", "User Activation");
				}
				
			}
			else {
				$this->_log->addError("Error: Failed to activate user (user_id=" . $this->_userId . "(" . mysqli_error($this->_dbconn) . ")", "User Activation");
			}
				
			
			$stmt->close();
			
		}
		
		return $accessToken;
		
	}
	
	public static function userExists($userName) {
		
		$exists=false;
		
		$query = "SELECT user_id FROM backup_user WHERE user_name = ?";
		if ( $stmt = mysqli_prepare(BackupDatabase::getDatabase()->getConnection(), $query) ) {
			
			$stmt->bind_param('s', $userName );
			if ( $stmt->execute() ) {
				$result = $stmt->get_result();
				if ( mysqli_num_rows($result) > 0 )
					$exists=true;
			}
			
			$stmt->close();
			
		}
		
		return $exists;
		
	}

}

?>