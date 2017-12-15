<?php

require_once 'database.class.php';
require_once 'session.class.php';

class BackupLog
{
	
	private static $factory;
	private $_dbconn;
	private $_session;
	private $_flush = TRUE;
	private $_userID;
	
	public function __construct() {
		
		//Connect to SQL database
		$this->_dbconn = BackupDatabase::getDatabase()->getConnection();
		
		//Get User Session
		$this->_session = BackupSession::getSession();		
		
		$this->_userID = $this->_session->getUserID();
		
	}
	
	function __destruct() {
		
	}
	
	public static function getLog()
	{
		if (!self::$factory)
			self::$factory = new BackupLog();
		
		return self::$factory;
	}
	
	public function addMessage($msg, $type='Info', $priority=0) {
		
		if ( $stmt = mysqli_prepare($this->_dbconn, "INSERT INTO backup_log (message,type,user_id,priority) VALUES(?,?,?,?)") ) {
			
			$stmt->bind_param('ssii', $msg, $type, $this->_userID, $priority );
			
			$stmt->execute();
			$stmt->close();
			
		}
		else {
			echo "Failed to add log message to database: " . mysqli_error($this->_dbconn) . "<br/>";
		}
		
		//Output to client?
		if ( $this->_flush ) {
			echo $msg . "<br/>";
			flush();
		}
		
	}
	
}

?>