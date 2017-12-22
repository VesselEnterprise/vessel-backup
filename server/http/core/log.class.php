<?php

require_once 'database.class.php';
require_once 'session.class.php';

class BackupLog
{
	
	private static $factory;
	private $_dbconn;
	private $_session;
	private $_flush = false;
	private $_userID = -1;
	
	public function __construct() {
		
		//Connect to SQL database
		$this->_dbconn = BackupDatabase::getDatabase()->getConnection();
		
	}
	
	function __destruct() {
		
	}
	
	public static function getLog()
	{
		if (!self::$factory)
			self::$factory = new BackupLog();
		
		return self::$factory;
	}
	
	public function setOutput($flag) {
		$this->_flush = $flag;
	}
	
	public function addError($msg, $type='Info', $priority=0, $output=false ) {
		
		$this->addMessage( $msg, $type, $priority, $output, 1 );
		
	}
	
	public function addMessage($msg, $type='Info', $priority=0, $output=false, $error=0) {
		
		$userID = BackupSession::getSession()->getUserID();
		
		if ( $stmt = mysqli_prepare($this->_dbconn, "INSERT INTO backup_log (message,type,user_id,priority,error) VALUES(?,?,?,?,?)") ) {
			
			$stmt->bind_param('ssiii', $msg, $type, $userID, $priority, $error );
			
			$stmt->execute();
			$stmt->close();
			
		}
		else {
			echo "Failed to add log message to database: " . mysqli_error($this->_dbconn) . "<br/>";
		}
		
		//Output to client?
		if ( $this->_flush || $output ) {
			echo $msg . "<br/>";
			flush();
		}
		
	}
	
}

?>