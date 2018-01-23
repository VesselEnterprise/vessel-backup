<?php

require_once 'database.class.php';
require_once 'log.class.php';

class BackupFile
{
	
	private $_db;
	private $_dbconn;
	private $_fileId = -1;
	private $_uniqueId;
	private $_fileRow = array();
	private $_fileExists=false;
	private $_userId = -1;
	
	public function __construct($fileId=-1) {
		
		$this->_db = BackupDatabase::getDatabase();
		$this->_dbconn = $this->_db->getConnection();
		$this->_log = BackupLog::getLog();
		
		if ( $fileId >= 0 ) {
			$this->_getFileById($fileId);
		}
		
	}
	
	public function getFileById($fileId) {
		
		$this->_fileRow=null;
		
		$this->_getFileById($fileId);
		return $this->_fileRow;
	}
	
	public function getFileByUniqueId($uniqueId) {
		
		$this->_fileRow=null;
		
		$this->_getFileByUniqueId($uniqueId);
		
		return $this->_fileRow;
			
	}
	
	private function _getFileById($fileId) {
		
		$query = "SELECT * FROM backup_file WHERE file_id=" . $fileId;
		
		if ( $this->_userId > 0 )
			$query .= " AND user_id=" . $this->_userId;
		
		if ( $result = mysqli_query($this->_dbconn,$query ) ) {
			
			if ( $this->_fileRow = mysqli_fetch_assoc($result) ) {
				
				$this->_fileId = $fileId;
				$this->_uniqueId = $this->_fileRow['unique_id'];
				$this->_fileExists = true;
				
			}
			
			$result->close();
		}
		
	}
	
	private function _getFileByUniqueId($uniqueId) {
		
		$query = "SELECT * FROM backup_file WHERE unique_id='" . $uniqueId . "'";
		
		if ( $this->_userId > 0 )
			$query .= " AND user_id=" . $this->_userId;
		
		if ( $result = mysqli_query($this->_dbconn, $query) ) {
			
			if ( $this->_fileRow = mysqli_fetch_assoc($result) ) {
				
				$this->_fileId = $this->_fileRow['file_id'];
				$this->_uniqueId = $uniqueId;
				$this->_fileExists = true;
				
			}
			else
				$this->_fileExists=false;
			
			$result->close();
		}
		else {
			$this->_log->addError("Failed to query for backup_file: " . mysqli_error($this->_dbconn), "BackupFile");
			$this->_fileExists = false;
		}
		
	}
	
	/** This will limit the scope of file queries in this class to the given user id **/
	public function setUserId($userId) {
		$this->_userId = $userId;	
	}
	
	public function getFile() {
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