<?php

require_once 'database.class.php';
require_once 'session.class.php';

class BackupLog
{

	private static $factory;
	private $_dbconn;
	private $_session;
	private $_flush = false;
	private $_userId = null;

	public function __construct($userId) {

		//Connect to SQL database
		$this->_dbconn = BackupDatabase::getDatabase()->getConnection();

		$this->_userId = $userId;

	}

	function __destruct() {

	}

	public static function getLog($userId=null)
	{
		if (!self::$factory)
			self::$factory = new BackupLog($userId);

		return self::$factory;
	}

	public function setUserId($userId) {
		$this->_userId = $userId;
	}

	public function setOutput($flag) {
		$this->_flush = $flag;
	}

	public function addError($msg, $type='Info', $priority=0, $output=false ) {

		$this->addMessage( $msg, $type, $priority, $output, 1 );

	}

	public function addMessage($msg, $type='Info', $priority=0, $output=false, $error=0) {

		if ( $stmt = mysqli_prepare($this->_dbconn, "INSERT INTO backup_log (message,type,user_id,priority,error) VALUES(?,?,UNHEX(?),?,?)") ) {

			$stmt->bind_param('sssii', $msg, $type, $this->_userId, $priority, $error );

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
