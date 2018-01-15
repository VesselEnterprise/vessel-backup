<?php

require_once 'database.class.php';

class BackupFile
{
	
	private $_db;
	private $_fileID = -1;
	private $_fileRow = array();
	private $_fileExists=false;
	
	public function __construct($fileID) {
		
		$this->_db = BackupDatabase::getDatabase();
		
		$this->_fileID = $fileID;
		
		$this->_getFileRow($this->_fileID);	
		
	}
	
	private function _getFileRow() {
		
		if ( $result = mysqli_query($this->_db->getConnection(), "SELECT * FROM backup_file WHERE file_id=" . $this->_fileID) ) {
			
			if ( $this->_fileRow = mysqli_fetch_assoc($result) ) {
			
				//Fix data types
				for ( $i=0; $i < sizeof($this->_fileRow); $i++ ) {

					$metadata = $result->fetch_field_direct($i);

					$this->_fileRow[$metadata->name] = $metadata->type == 3 ? (int)$this->_fileRow[$metadata->name] : (string)$this->_fileRow[$metadata->name];

				}
				
				$this->_fileExists = true;
				
			}
			
			$result->close();
		}		
		
	}
	
	public function getFile() {
		return $this->_fileRow;	
	}
	
	public function exists() {
		return $this->_fileExists;	
	}

}

?>