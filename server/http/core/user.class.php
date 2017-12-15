<?php

require_once 'database.class.php';

class BackupUser
{
	
	private static $factory;
	
	private $_userID;
	private $_dbconn;
	private $_log;
	
	public function __construct($userID) {
		
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
		
		$hash = password_hash( $pwd );
		
		//Update user password
		if ( $stmt = mysqli_prepare("UPDATE backup_user SET password=?,password_set=NOW() WHERE userID=?") ) {
			
			$stmt->bind_param('si', $hash, $this->_userID );
			$stmt->execute();
			$stmt->close();
			
			$this->_log->addMessage("User (User ID = " . $this->_userID . ") changed their password");
			
		}		
		
	}

}

?>