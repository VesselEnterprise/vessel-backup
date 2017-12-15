<?php

require_once 'database.class.php';
require_once 'session.class.php';

class BackupLog
{
	
	private static $factory;
	private $_dbconn;
	private $_session;
	
	public function __construct() {
		
		//Connect to SQL database
		$this->_dbconn = BackupDatabase::getFactory()->getConnection();
		
		//Get User Session
		$this->_session = BackupSession::getSession();		
		
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
		
		if ( $stmt = mysqli_prepare("INSERT INTO backup_log (message,type,user_id,priority) VALUES(?,?,?,?)") ) {
			
			$stmt->bind_param('ssii', $msg, $type, $_session->getUserID(), $priority );
			
			$stmt->execute();
			$stmt->close();
			
		}
		
	}
	
}

?>