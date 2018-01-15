<?php

require_once 'abstract_upload.class.php';

class BackupUpload extends Upload
{

	public function __construct($fileMetadata, &$fileContent) {
		
		parent::__construct();
		
		$this->_metadata = $fileMetadata;
		$this->_contentBytes = $fileContent;
		
		$this->_isValid = $this->_parseFileData();
		
	}
	
	private function _parseFileData() {
		
		$requiredFields = array(
			'file_name',
			'file_size',
			'file_type',
			'hash',
			'file_path',
			'last_modified',
			//'data',
			'compressed'
		);
		
		foreach ( $requiredFields as $key => $val ) {
			
			$fv = $this->_metadata->{$val};
			
			if ( empty($fv) && gettype($fv) != "boolean" ) {
				$this->_setError("Required field is missing: " . $val);
				return false;
			}
			
		}
		
		//$this->_contentBytes = base64_decode( $this->_metadata->{'data'} );
		//$this->_metadata->{'data'} = null; //Free memory
		
		//Verify SHA-1 hash
		$hashed = sha1($this->_contentBytes);
		
		if ( $hashed != strtolower($this->_metadata->{'hash'}) ) {
			$this->_setError("File contents integrity check failed. SHA-1 hash mismatch- the file contents are not correct.");
			return false;
		}
		
		return true;
		
	}
	
	/**
	* For single file uploads
	*/
	public function upload() {
		
		if ( !$this->_isValid ) {
			$this->_setError("Could not complete upload. The payload is invalid");
			return false;
		}
		
		//Insert a new file
		$query = "INSERT INTO backup_file (unique_id,file_name,file_size,user_id,file_type,hash,file_path,last_modified,last_backup) VALUES(?,?,?,?,?,?,?,FROM_UNIXTIME(?),NOW()) ON DUPLICATE KEY UPDATE file_id=LAST_INSERT_ID(file_id),file_size=?,last_modified=FROM_UNIXTIME(?),last_backup=NOW()";
		if ( $stmt = mysqli_prepare($this->_db->getConnection(), $query ) ) {
			
			$uniqueID = sha1( $this->_metadata->{'file_path'} . $this->_metadata->{'file_name'} );
			
			$stmt->bind_param(
				'ssiisssiii',
				$uniqueID,
				$this->_metadata->{'file_name'},
				$this->_metadata->{'file_size'},
				$this->_userID,
				$this->_metadata->{'file_type'},
				$this->_metadata->{'hash'},
				$this->_metadata->{'file_path'},
				$this->_metadata->{'last_modified'},
				$this->_metadata->{'file_size'},
				$this->_metadata->{'last_modified'}
			);
			
			if ( $stmt->execute() ) {
				$this->_fileID = mysqli_insert_id( $this->_db->getConnection() );
			}
			else {
				$this->_setError("There was an error inserting the file. (" . mysqli_error($this->_db->getConnection()) . ")");
				return false;
			}
			
			$stmt->close();
			
		}
		
		if ( $this->_fileID < 0 ) {
			$this->_setError("There was an error inserting the file. (" . mysqli_error($this->_db->getConnection()) . ")");
			return false;
		}
		
		//Create new upload
		$query = "INSERT INTO backup_upload (file_id,user_id,parts,bytes,hash) VALUES(?,?,1,?,?)";
		if ( $stmt = mysqli_prepare($this->_db->getConnection(), $query ) ) {
		
			$stmt->bind_param('iiis', $this->_fileID, $this->_userID, $this->_metadata->{'file_size'}, $this->_metadata->{'hash'} );
			
			if ( $stmt->execute() ) {
				
				$this->_uploadID = mysqli_insert_id( $this->_db->getConnection() );
				
			}
			
			$stmt->close();
			
		}
		
		if ( $this->_uploadID < 0 ) {
			$this->_setError("There was an error creating the file upload (" . mysqli_error($this->_db->getConnection()) . ")");
			return false;
		}
		
		if ( !$this->_write_file() ) {
			$this->_setError("Failed to write file to local storage. Please check the path and folder permissions (" . $this->getError() . ")");
			return false;
		}
		
		return true;
		
	}
	
	private function _write_file() {
		
		$target = $this->_getTarget();
		
		if ( $target['type'] == "local") {
			
			return $this->_write_file_local($target['path']);
			
		}
		
		return true;
		
	}
	
	private function _write_file_local($path) {
		
		$base_dir = $path . "/users/" . $this->_userID . "/" . $this->_metadata->{'file_path'};
		$target_path = $base_dir . "/" . $this->_metadata->{'file_name'};
		
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
	
}

?>