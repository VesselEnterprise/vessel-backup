<?php

require_once 'database.class.php';
require_once 'api_session.class.php';
require_once 'file.class.php';

class BackupUpload
{
	
	/**
	 ** Private members
	**/
	private $_db;
	private $_errorMsg;
	private $_uploadId = -1;
	private $_fileId = -1;
	private $_userId = -1;
	private $_uploadRow;
	private $_backupTarget;
	private $_isValid = false;
	private $_metadata;
	private $_contentBytes;

	public function __construct($metadata) {
		
		$this->_db = BackupDatabase::getDatabase();
		$this->_session = BackupAPISession::getSession();
		$this->_userId = $this->_session->getUserId();
		$this->_metadata = $metadata;
		
	}
	
	private function _setError($msg) {
		$this->_errorMsg = $msg;
		$this->_isValid=false;
	}
	
	public function isValid() {
		return $this->_isValid;	
	}
	
	public function getError() {
		return $this->_errorMsg;	
	}
	
	public function getFileId() {
		return $this->_fileId;	
	}
	
	public function getUploadId() {
		return $this->_uploadId;	
	}
	
	/**
	  * Gets a backup_upload record and sets some internal member vars
	**/
	private function _getUpload($uploadId) {
		
		$query = "SELECT * FROM backup_upload WHERE upload_id=?";
		if ( $stmt = mysqli_prepare($this->_db->getConnection(), $query) ) {
			
			$stmt->bind_param('i', $uploadId);
			if ( $stmt->execute() ) {
				
				$result = $stmt->get_result();
				$this->_uploadRow = $result->fetch_assoc();
				
				//Set internal member vars
				$this->_uploadId = $this->_uploadRow['upload_id'];
				$this->_fileId = $this->_uploadRow['file_id'];
				
			}
			else {
				$this->_setError("There was an error inserting the file. (" . mysqli_error($this->_db->getConnection()) . ")");
				return false;
			}
			
			$stmt->close();
			
		}
		
		return $this->_uploadRow;
		
	}
	
	public function getUpload() {
		return $this->_uploadRow;	
	}
	
	public function setUploadId($uploadId) {
		$this->_getUpload($uploadId);
	}
	
	/* Initialize a new file upload and return the upload Id to the client */
	public function init_upload() {
		
		$requiredFields = array(
			'file_name',
			'file_size',
			'file_type',
			'hash',
			'file_path',
			'parts',
			'last_modified'
			//compressed
		);
		
		//Verify required fields exist
		foreach ( $requiredFields as $key => $val ) {
			
			$fv = $this->_metadata->{$val};
			
			if ( empty($fv) && gettype($fv) != "boolean" ) {
				$this->_setError("Required field is missing: " . $val);
				return false;
			}
			
		}
		
		//The database unique ID is the SHA-1 hash of the file path
		$uniqueId = sha1( $this->_metadata->{'file_path'} . "/" . $this->_metadata->{'file_name'} );
		
		//Check and see if the file already exists in the database
		//If the hash is unchanged AND it exists, cancel the upload
		$tmpFile = new BackupFile();
		$tmpFile->setUserId($this->_userId); //Set user scope
		$tmpFile->getFileByUniqueId($uniqueId);
		if ( $tmpFile->exists() ) {
			if ( $tmpFile->getValue['hash'] == $this->_metadata->{'hash'} && ($tmpFile->getValue['uploaded'] == 1) ) {
				$this->_setError("File has already been uploaded");
				return false;
			}
		}
		
		//Insert a new file
		$query = "INSERT INTO backup_file (unique_id,file_name,file_size,user_id,file_type,hash,file_path,last_modified,last_backup) VALUES(?,?,?,?,?,?,?,FROM_UNIXTIME(?),NOW()) ON DUPLICATE KEY UPDATE file_id=LAST_INSERT_ID(file_id),file_size=?,last_modified=FROM_UNIXTIME(?),last_backup=NOW()";
		if ( $stmt = mysqli_prepare($this->_db->getConnection(), $query ) ) {

			$stmt->bind_param(
				'ssiisssiii',
				$uniqueId,
				$this->_metadata->{'file_name'},
				$this->_metadata->{'file_size'},
				$this->_userId,
				$this->_metadata->{'file_type'},
				$this->_metadata->{'hash'},
				$this->_metadata->{'file_path'},
				$this->_metadata->{'last_modified'},
				$this->_metadata->{'file_size'},
				$this->_metadata->{'last_modified'}
			);
			
			if ( $stmt->execute() ) {
				$this->_fileId = mysqli_insert_id( $this->_db->getConnection() );
			}
			else {
				$this->_setError("There was an error inserting the file. (" . mysqli_error($this->_db->getConnection()) . ")");
				return false;
			}
			
			$stmt->close();
			
		}
		
		if ( $this->_fileId < 0 ) {
			$this->_setError("There was an error inserting the file. (" . mysqli_error($this->_db->getConnection()) . ")");
			return false;
		}
		
		//Create new upload and associate it with the file
		$query = "INSERT INTO backup_upload (file_id,user_id,parts,bytes,hash) VALUES(?,?,?,?,?)";
		if ( $stmt = mysqli_prepare($this->_db->getConnection(), $query ) ) {
		
			$stmt->bind_param('iiiis', $this->_fileId, $this->_userId, $this->_metadata->{'parts'}, $this->_metadata->{'file_size'}, $this->_metadata->{'hash'} );
			
			if ( $stmt->execute() ) {
				$this->_uploadId = mysqli_insert_id( $this->_db->getConnection() );
			}
			
			$stmt->close();
			
		}
		
		if ( $this->_uploadId < 0 ) {
			$this->_setError("There was an error creating the file upload (" . mysqli_error($this->_db->getConnection()) . ")");
			return false;
		}
		
		return $this->_uploadId;
		
	}
	
	/**
	* Upload file contents
	*/
	public function upload_part($contents) {
		
		if ( empty($contents) ) {
			$this->_setError("File contents are empty");
			return false;
		}
		
		$requiredFields = array(
			'upload_id',
			'part_number',
			'part_size',
			'hash'
		);
		
		//Verify required fields exist
		foreach ( $requiredFields as $key => $val ) {
			
			$fv = $this->_metadata->{$val};
			
			if ( empty($fv) && gettype($fv) != "boolean" ) {
				$this->_setError("Required field is missing: " . $val);
				return false;
			}
			
		}
		
		//Verify Upload Exists
		$this->setUploadId($this->_metadata->{'upload_id'});
		$fileUpload = $this->getUpload();
		if ( !$fileUpload ) {
			$this->_setError("File upload with id " . $this->_metadata->{'upload_id'} . " does not exist");
			return false;
		}
		
		//Verify SHA-1 hash
		$contentHash = sha1($contents);
		
		if ( $contentHash != strtolower($this->_metadata->{'hash'}) ) {
			$this->_setError("File contents integrity check failed. SHA-1 hash mismatch- the file contents are not correct.");
			return false;
		}
		
		//Verify part size
		$contentSize = strlen($contents);
		if ( $this->_metadata->{'part_size'} != $contentSize ) {
			$this->_setError("File part error: Total byte size is not correct.");
			return false;
		}
		
		//Set internal file contents after they have been verified
		$this->_contentBytes = $contents;
		
		//Get the associated backup file
		$file = new BackupFile($this->_fileId);
		if ( !$file->exists() ) {
			$this->_setError("File with id " . $this->_fileId . " does not exist");
			return;
		}
		
		//Check for valid part number
		if ( $this->_metadata->{'part_number'} > $fileUpload['parts'] ) {
			$this->_setError("Invalid part number was specified.");
			return false;
		}
		
		//Verify Backup Target
		$target = $this->_getTarget();
		
		if ( !$target ) {
			$this->_setError("Could not find storage target");
			return false;
		}
		
		//Write file content to storage
		//Storage Target Handlers here
		if ( $fileUpload['parts'] <= 1 )
			$fileName = $file->getValue('file_name');
		else
			$fileName = $file->getValue('unique_id') . "_" . $this->_metadata->{'part_number'} . ".part";
		
		if ( $target['type'] == "local") {
			
			//If there is only one part, write to the file path. If there are multiple parts, we write to the parts folder
			$filePath = $target['path'];
			
			if ( $fileUpload['parts'] <= 1 )
				$filePath .= "/users/" . $this->_userId . $file->getValue('file_path');
			else
				$filePath .= "/parts";
			
			$filePath .= "/" . $fileName;
			
		
			if ( !$this->_writeLocalFile($target['path']) ) {
				$this->_setError("Failed to write file to local storage. Please check the path and folder permissions (" . $this->getError() . ")");
				return false;
			}
			
		}
		
		//Add a new part to the database if the write operation was successful
		$partId = -1;
		$query = "REPLACE INTO backup_upload_part (upload_id,part_number,bytes,tmp_file_name,hash) VALUES(?,?,?,?,?)";
		if ( $stmt = mysqli_prepare($this->_db->getConnection(), $query) ) {
			
			$stmt->bind_param(
				'iiiss',
				$this->_uploadId,
				$this->_metadata->{'part_number'},
				$contentSize,
				$tmpFileName,
				$contentHash
			);
			
			if ( $stmt->execute() ) {
				$partId = mysqli_insert_id($this->_db->getConnection());
			}
				
			$stmt->close();
			
		}
		
		if ( $partId < 0 ) {
			$this->_setError("Failed to add upload part to the database (" . mysqli_error($this->_db->getConnection()) . ")");
			return false;
		}

		//If total parts = 1, automatically complete the upload
		if ( $fileUpload['parts'] == 1 )
			$this->completeUpload($this->_uploadId);
		
		return $partId;
		
	}
	
	public function completeUpload($uploadId) {
		
		if ( !isset($this->_uploadRow) ) {
			$this->
		}

	}
	
	private function _writeLocalFile($path) {
		
		$file = new BackupFile($this->_fileId);
		
		if ( !$file->exists() ) {
			$this->_setError("File with id " . $this->_fileId . " does not exist");
			return false;
		}
		
		$base_dir = $path . "/users/" . $this->_userId . "/" . $file->getData['file_path'];
		$target_path = $base_dir . "/" . $file->getData['file_name'];
		
		//Check if directory exists, if not then create it
		if ( !is_dir($base_dir) ) {
			if ( mkdir($base_dir, 0777, true) ) {
				chmod($base_dir, 0777);
			}
			else {
				$this->_setError("Failed to create directory or set directory permissions: " . $target_path);
				return false;
			}
			
		}
		
		//Try to CHMOD the dir if not writable before failing completely
		if ( !is_writable($base_dir) ) {
			chmod($base_dir, 0777);
		}
		
		//Save file
		if ( is_writable($base_dir) ) {
			
			if ( $fh = fopen($target_path, 'w') ) {
				
				if ( !fwrite($fh, $this->_contentBytes) ) {
					$this->_setError("Could not write to file " . $target_path);
					return false;
				}
				
				fclose($fh);
				
			}
			else {
				$this->_setError("The file handle could not be obtained: " . $target_path);
				return false;
			}
			
		}
		else {
			$this->_setError("The file is not writable: " . $target_path);
			return false;
		}
			
		return true;		

	}
	
	private function _getTarget() {

		$backupTarget = NULL;
		$hasUserTarget = false;

		//Check if there are any individual user targets configured
		$query = "SELECT a.* FROM backup_target AS a INNER JOIN backup_user_target AS b ON a.target_id=b.target_id WHERE b.user_id=" . $this->_userId;
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