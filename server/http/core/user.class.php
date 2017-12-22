<?php

require_once 'database.class.php';

class BackupUser
{
	
	private static $factory;
	
	private $_userID;
	private $_dbconn;
	private $_log;
	private $_userRow;
	
	public function __construct($userID) {
		
		$this->_userID = $userID;
		
		//Get database connection
		$this->_dbconn = BackupDatabase::getDatabase()->getConnection();
		
		//Get Log Object
		$this->_log = BackupLog::getLog();
		
		$this->_userRow = $this->_getUserRow($userID);
		
	}
	
	function __destruct() {
		
	}
	
	private function _getUserRow($userID) {
		
		$row = array();
		
		$userName = (string)$userID;
		
		$query = "SELECT * FROM backup_user WHERE user_id=? OR user_name=?";
		if ( $stmt = mysqli_prepare($this->_dbconn, $query ) ) {
			$stmt->bind_param('is', $userID, $userName );
			if ( $stmt->execute() ) {
				$result = $stmt->get_result();
				$row = mysqli_fetch_array($result);
			}
			$stmt->close();
		}
		
		//Check if user_name was provided instead
		if ( gettype($userID) == "string" )
			$this->_userID = $row['user_id'];
		
		return $row;		
		
	}
	
	public function getUserData($col) {
		return $this->_userRow[$col];
	}
	
	public static function getUser($userID)
	{
		if (!self::$factory)
			self::$factory = new BackupUser($userID);
		
		return self::$factory;
	}
	
	public function setPassword($pwd) {
		
		$hash = password_hash( $pwd, PASSWORD_DEFAULT );
		
		//Update user password
		if ( $stmt = mysqli_prepare("UPDATE backup_user SET password=?,password_set=NOW() WHERE userID=?") ) {
			
			$stmt->bind_param('si', $hash, $this->_userID );
			$stmt->execute();
			$stmt->close();
			
			$this->_log->addMessage("User (User ID = " . $this->_userID . ") changed their password");
			
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
			
			$stmt->bind_param('is', $this->_userID, $code );
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
			
			$stmt->bind_param('is', $this->_userID, $activationCode );
			
			if ( $stmt->execute() ) {
				
				$result = $stmt->get_result();
				if ( $result->num_rows > 0 ) {
					
					//User is now activated
					$accessToken = $this->generateAccessToken();
					
					//Set the user's access key
					if ( $s2 = mysqli_prepare($this->_dbconn, "UPDATE backup_user SET access_token=? WHERE user_id=?") ) {
						$s2->bind_param('si', $accessToken, $this->_userID);
						if ( $s2->execute() ) {
							$this->_log->addMessage("Successfully activated user (user_id=" . $this->_userID . ")", "User Activation");
						}
						$s2->close();
					}
					
					//Remove the user activation record
					@mysqli_query($this->_dbconn, "DELETE FROM backup_user_activation WHERE user_id=" . $this->_userID );
					
				}
				else {
					$this->_log->addError("Error: Failed to activate user (user_id=" . $this->_userID . ") (" . mysqli_error($this->_dbconn) . ")", "User Activation");
				}
				
			}
			else {
				$this->_log->addError("Error: Failed to activate user (user_id=" . $this->_userID . "(" . mysqli_error($this->_dbconn) . ")", "User Activation");
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