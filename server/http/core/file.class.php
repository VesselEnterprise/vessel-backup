<?php

require_once 'database.class.php';
require_once 'log.class.php';

class BackupFile
{

	private $_db;
	private $_dbconn;
	private $_fileId = NULL;
	private $_fileRow = array();
	private $_fileExists=false;
	private $_userId = -1;

	public function __construct($fileId=NULL,$userId=-1) {

		$this->_db = BackupDatabase::getDatabase();
		$this->_dbconn = $this->_db->getConnection();
		$this->_log = BackupLog::getLog();

		if ( !empty($fileId) && $userId > 0 ) {
			$this->queryFile($fileId,$userId);
		}

	}

	public function queryFile($fileId, $userId) {

		$query = "SELECT * FROM backup_file WHERE file_id=UNHEX(?) AND user_id=?";
		if ( $stmt = mysqli_prepare($this->_dbconn, $query) ) {

			$stmt->bind_param('si', $fileId, $userId);
			if ( $stmt->execute() ) {

				$result = $stmt->get_result();

				if ( $this->_fileRow = $result->fetch_assoc() ) {
					$this->_fileId = $fileId;
					$this->_fileExists = true;
					$this->_fileRow['file_id'] = bin2hex($this->_fileRow['file_id']);
					$this->_fileRow['hash'] = bin2hex($this->_fileRow['hash']);
				}
				else {
					$this->_fileExists = false;
				}

			}

			$result->close();
		}
		else {
			$this->_log->addError("Failed to query for backup_file: " . mysqli_error($this->_dbconn), "BackupFile");
			$this->_fileExists = false;
		}

		return $this->_fileExists;

	}

	public function getObject() {
		return $this->_fileRow;
	}

	public function exists() {
		return $this->_fileExists;
	}

	public function getValue($name) {
		return $this->_fileRow[$name];
	}

}

?>
