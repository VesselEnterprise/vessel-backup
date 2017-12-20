<?php

require_once 'database.class.php';

class BackupUser
{
	
	private static $factory;
	
	private $_userID;
	private $_dbconn;
	private $_log;
	
	public function __construct($userID=-1) {
		
		$this->_user_id = $userID;
		
		//Get database connection
		$this->_dbconn = BackupDatabase::getDatabase()->getConnection();
		
		//Get Log Object
		$this->_log = BackupLog::getLog();
		
	}
	
	function __destruct() {
		
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
	
	public function generateAccessToken() {
		return bin2hex(random_bytes(32));
	}
	
	public function activateUser($userName, $activationCode) {
		
		$accessToken = '';
		
		$query = "SELECT user_id FROM backup_user_activation AS a LEFT JOIN backup_user AS b ON a.user_id=b.user_id WHERE b.user_name = ? AND a.activation_code=?";
		
		if ( $stmt = mysqli_prepare($this->_dbconn, $query ) ) {
			
			$stmt->bind_param('ss', $userName, $activationCode );
			
			if ( $stmt->execute() ) {
				
				$result = $stmt->get_result();
				if ( $result->num_rows > 0 ) {
					
					//User is activated
					$accessToken = $this->generateAccessToken();
					
					//Set the user's access key
					if ( @mysqli_query($this->_dbconn, "UPDATE backup_user SET access_token = '" . $accessToken . "'") ) {
						$this->_log->addMessage("Successfully activated user " . $userName, "User Activation");
					}				
					
				}
				else {
					$this->_log->addMessage("Failed to activate user " . $userName, "User Activation");
				}
				
			}
			
			$stmt->close();
			
		}
		
		return $accessToken;
		
	}

}

?>