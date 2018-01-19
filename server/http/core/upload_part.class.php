<?php

	require_once 'abstract_upload.class.php';
	
	class BackupUploadPart extends Upload
	{
		
		private $_partFilename;
		
		public function __construct () {
			
			parent::__construct();
			
		}
		
		/**
		** Initializes a new MultiPart upload
		** Returns the upload_id to the client
		*/
		public function initialize($metadata) {
			
			$requiredFields = array(
				'file_name',
				'file_size',
				'file_type',
				'hash',
				'total_parts',
				'file_path',
				'last_modified',
			);
			
			//Verify required fields
			foreach ( $requiredFields as $key => $val ) {
			
				$fv = $metadata->{$val};

				if ( empty($fv) && gettype($fv) != "boolean" ) {
					$this->_setError("Required field is missing: " . $val);
					return false;
				}
				
			}
			
			
			
			//Insert a new backup_file
			
			$query = "INSERT INTO backup_file (unique_id,file_name,file_size,user_id,file_type,hash,file_path,last_modified,last_backup) VALUES(?,?,?,?,?,?,?,FROM_UNIXTIME(?),NOW()) ON DUPLICATE KEY UPDATE file_id=LAST_INSERT_ID(file_id),file_size=?,last_modified=FROM_UNIXTIME(?),last_backup=NOW()";
			
			if ( $stmt = mysqli_prepare($this->_db->getConnection(), $query ) ) {
			
				$uniqueID = sha1( $metadata->{'file_path'} . $metadata->{'file_name'} );

				$stmt->bind_param(
					'ssiisssiii',
					$uniqueID,
					$metadata->{'file_name'},
					$metadata->{'file_size'},
					$this->_userID,
					$metadata->{'file_type'},
					$metadata->{'hash'},
					$metadata->{'file_path'},
					$metadata->{'last_modified'},
					$metadata->{'file_size'},
					$metadata->{'last_modified'}
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
			
			//Insert a new upload
			$query = "INSERT INTO backup_upload (file_id,user_id,parts,bytes,hash) VALUES(?,?,?,?,?)";
			
			if ( $stmt = mysqli_prepare($this->_db->getConnection(), $query) ) {
				
				$stmt->bind_param(
					'iiiis',
					$this->_fileID,
					$this->_userID,
					$metadata->{'total_parts'},
					$metadata->{'file_size'},
					$metadata->{'hash'}
				);
				
				if ( $stmt->execute() ) {
					$this->_uploadID = mysqli_insert_id( $this->_db->getConnection() );	
				}
				
				$stmt->close();
				
			}
			
			if ( $this->_uploadID < 0 ) {
				$this->_setError("There was an error creating the file upload (" . mysqli_error($this->_db->getConnection()) . ")");
				return false;
			}
			
			return $this->_uploadID;

		}
		
		public function uploadPart($metadata, $contents) {
			
			$requiredFields = array(
				'upload_id',
				'part_number',
				'part_size',
				'hash'
			);
			
			//Verify required fields
			foreach ( $requiredFields as $key => $val ) {
			
				$fv = $this->_metadata->{$val};

				if ( empty($fv) && gettype($fv) != "boolean" ) {
					$this->_setError("Required field is missing: " . $val);
					return false;
				}
				
			}
			
			//Verify content is not null
			if ( empty($contents) ) {
				$this->_setError("No file content was provided");
				return false;
			}
			
			//Verify content hash
			$hashed = sha1( $contents );
			if ( strtolower($metadata->{'hash'}) != $hashed ) {
				$this->_setError("File contents integrity check failed. SHA-1 hash mismatch- the file contents are not correct.");
				return false;
			}

			//Verify part size
			$contentSize = strlen($contents);
			if ( $metadata->{'part_size'} != $contentSize ) {
				$this->_setError("File part error: Total byte size is not correct.");
				return false;
			}
			
			//Get File Data
			$query = "SELECT a.file_id,a.parts,a.upload_id,b.unique_id FROM backup_upload AS a WHERE a.upload_id=? AND a.user_id=? INNER JOIN backup_file AS b ON a.file_id=b.file_id";
			
			if ( $stmt = mysqli_prepare($this->_db->getConnection(), $query ) ) {
				
				$stmt->bind_param('ii', $metadata->{'upload_id'}, $this->_userID );
				
				if ( $stmt->execute() ) {
					
					$result = $stmt->get_result();
					if ( $row = $result->fetch_assoc() ) {
						
						//Set class vars
						$this->_uploadID = $row['upload_id'];
						$this->_fileID = $row['file_id'];
						
						//Check for valid part number
						if ( $metadata->{'part_number'} > $row['parts'] ) {
							$this->_setError("Invalid part number was specified.");
							$stmt->close();
							return false;
						}
						
						//eg. xxxxxxxxxx_1.part
						$this->_partFilename = $row['unique_id'] . "_" . $metadata->{'part_number'} . ".part";
					
						//Add parts to database
						$part_query = "REPLACE INTO backup_upload_part (upload_id,part_number,bytes,tmp_file_name,hash) VALUES(?,?,?,?,?)";

						if ( $part_stmt = mysqli_prepare( $this->_db->getConnection(), $part_query) ) {
							$part_stmt->bind_param(
								'iiiss',
								$metadata->{'upload_id'},
								$metadata->{'part_number'},
								$contentSize,
								$this->_partFilename,
								$hashed
							);
							
							if ( $part_stmt->execute() ) {
								
								$this->_fileContentsRaw = $contents; //Copy to base class
								unset($contents); //Free memory
								
								//Write part to storage
								if ( !$this->_writeFilePart() ) {
									$this->_setError("Failed to write file part to storage");
									return false;
								}
								
							}
							
							$part_stmt->close();

						}
								
					}
					
				}
				
				$stmt->close();

			}

		}
				
		private function _writeFilePart() {
			
			$target = $this->_getTarget();
		
			if ( $target['type'] == "local") {

				return $this->_writePartLocal($target['path']);

			}

			return true;

		}
				
		private function _writePartLocal($path) {
			
			$base_dir = $path . "/parts";
			$target_path = $base_dir . "/" . $this->_partFilename;
		
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