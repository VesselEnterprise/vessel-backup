<?php

require_once 'database.class.php';
require_once 'log.class.php';
require_once 'user.class.php';

class BackupSession
{
	
	private static $factory;
	private $_userID;
	private $_dbconn;
	
	public function __construct() {
		
		//Get Database Connection
		$this->dbconn = BackupDatabase::getFactory()->getConnection();
		
		//Get User
		
		session_start();	
		
	}
	
	function __destruct() {
		
		
		session_destroy();
		
	}
	
	public static function getSession()
	{
		if (!self::$factory)
			self::$factory = new BackupSession();
		
		return self::$factory;
	}
	
	public function login($username,$pwd) {
		
		//Get User Password Hash
		if ( $stmt = mysqli_query($this->_dbconn, "SELECT user_id, password FROM backup_user WHERE user_name=?") ) {
			
			$stmt->bind_param('s', $username);
			$stmt->execute();
			
			$result = $stmt->get_result();
			
			if ( $row = $result->fetch_row() ) {
				
				if ( password_verify($pwd, $row[1]) == TRUE ) {
					
					//Password is correct, login successful
					$this->_updateLastLogin();
					
				}
				
			}
			
			$stmt->close();
			
		}

	}
	
	public function logout() {
		
		session_unset();
		self::__destruct();
	}
	
	public function getUserID() {
		return $this->_userID;
	}
	
	private function updateLastLogin() {
		
		if ( !$this->_userID )
			return;
		
		mysqli_query($this->_dbconn, "UPDATE backup_user SET last_login = NOW() WHERE user_id = " . $this->_userID);
		
	}
	
}

?>