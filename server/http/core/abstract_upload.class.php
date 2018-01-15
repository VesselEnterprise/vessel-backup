<?php

	require_once 'database.class.php';
	require_once 'api_session.class.php';

	//Abstract class for File uploads
	abstract class Upload
	{
		
		protected $_db;
		protected $_errorMsg;
		protected $_isValid = false;
		protected $_fileID = -1;
		protected $_metadata;
		protected $_userID = -1;
		protected $_backupTarget = array();
		protected $_uploadID = -1;
		protected $_contentBytes;
		
		function __construct() {
			$this->_db = BackupDatabase::getDatabase();
			$this->_session = BackupAPISession::getSession();
			$this->_userID = $this->_session->getUserID();	
		}
		
		protected function _setError($msg) {
			$this->_errorMsg = $msg;
			$this->_isValid=false;
		}

		public function isValid() {
			return $this->_isValid;	
		}

		public function getError() {
			return $this->_errorMsg;	
		}

		public function getFileID() {
			return $this->_fileID;	
		}

		public function getUploadID() {
			return $this->_uploadID;	
		}
		
		protected function _getTarget() {
		
			$backupTarget=array();
			$hasUserTarget=false;

			//Check if there are any individual user targets configured
			$query = "SELECT a.* FROM backup_target AS a INNER JOIN backup_user_target AS b ON a.target_id=b.target_id WHERE b.user_id=" . $this->_userID;
			if ( $result = mysqli_query($this->_db->getConnection(), $query) ) {

				if ( $backupTarget = mysqli_fetch_array($result) ) {
					$hasUserTarget=true;
				}

				$result->close();

			}

			if ( $hasUserTarget ) {
				return $backupTarget;
			}

			//Find the default Global target
			$query = "SELECT a.* FROM backup_target AS a INNER JOIN backup_setting AS b ON CAST(a.target_id AS CHAR(1)) = b.value WHERE b.name = 'default_target'";
			if ( $result = mysqli_query($this->_db->getConnection(), $query) ) {

				if ( $backupTarget = mysqli_fetch_array($result) ) {
					$hasUserTarget=false;
				}

				$result->close();

			}

			return $backupTarget;
		
		}
		
	}

?>