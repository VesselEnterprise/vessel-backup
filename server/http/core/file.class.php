<?php

require_once 'database.class.php';

class BackupFile
{
	
	private $_db;
	private $_fileId = -1;
	private $_uniqueId;
	private $_fileRow = array();
	private $_fileExists=false;
	private $_userId = -1;
	
	public function __construct($fileId=-1) {
		
		$this->_db = BackupDatabase::getDatabase();
		
		if ( $fileId >= 0 ) {
			$this->_getFileById($this->_fileId);
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
	
	private function _getFileById($id) {
		
		$query = "SELECT * FROM backup_file WHERE file_id=" . $fileId;
		
		if ( $this->_userId > 0 )
			$query .= " AND user_id=" . $this->_userId;
		
		if ( $result = mysqli_query($this->_db->getConnection(), ) ) {
			
			if ( $this->_fileRow = mysqli_fetch_assoc($result) ) {
				
				$this->_fileId = $fileId;
				$this->_uniqueId = $this->_fileRow['unique_id'];
				$this->_fileExists = true;
				
			}
			
			$result->close();
		}
		
	}
	
	private function _getFileByUniqueId($uniqueId) {
		
		$query = "SELECT * FROM backup_file WHERE unique_id=" . $uniqueId;
		
		if ( $this->_userId > 0 )
			$query .= " AND user_id=" . $this->_userId;
		
		if ( $result = mysqli_query($this->_db->getConnection(), $query) ) {
			
			if ( $this->_fileRow = mysqli_fetch_assoc($result) ) {
				
				$this->_fileId = $this->_fileRow['file_id'];
				$this->_uniqueId = $uniqueId;
				$this->_fileExists = true;
				
			}
			
			$result->close();
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