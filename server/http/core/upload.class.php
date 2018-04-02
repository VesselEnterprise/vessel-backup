<?php

require_once 'database.class.php';
require_once 'log.class.php';
require_once 'api_session.class.php';
require_once 'api_exception.class.php';
require_once 'file.class.php';

class BackupUpload
{

    /**
     ** Private members
    **/
    private $_db;
    private $_log;
    private $_dbconn;
    private $_errorMsg;
    private $_uploadId = -1;
    private $_fileId = null;
    private $_userId = -1;
    private $_uploadRow;         //Associated array of backup_upload row
    private $_file;              //BackupFile object
    private $_backupTarget;
    private $_metadata;
    private $_contentBytes;
    private $_uploadComplete=false;
    private $_partsUploaded=0;
    private $_partsRemaining=0;
    private $_totalParts=0;

    public function __construct($metadata)
    {
        $this->_db = BackupDatabase::getDatabase();
        $this->_dbconn = $this->_db->getConnection();
        $this->_session = BackupAPISession::getSession();
        $this->_userId = $this->_session->getUserId();
        $this->_log = BackupLog::getLog($this->_userId);
        $this->_metadata = $metadata;

        //If upload id exists in the metadata, preload the upload row
        if (isset($metadata->{'file_id'})) {
            $this->_fileId = $metadata->{'file_id'};
            $this->_getUpload($metadata->{'file_id'});
        }

        //If upload row was found, pre-load the file data
        if ($this->_uploadId > -1) {
            if ( !$this->_file = new BackupFile($this->_uploadRow['file_id'], $this->_userId) ) {
							throw new APIException("Invalid upload id was specified. File id does not exist");
						}
        }

        //Preload storage target
        $this->_backupTarget = $this->_getTarget();
    }

    private function _setError($msg)
    {

        //Update database log
        $this->_log->addError($msg, "API");

        //Set internal error message
        $this->_errorMsg = $msg;
    }

    public function getError()
    {
        return $this->_errorMsg;
    }

    public function getFileId()
    {
        return $this->_fileId;
    }

    public function getUploadId()
    {
        return $this->_uploadId;
    }

    /**
      * Gets a backup_upload record and sets some internal member vars
    **/
    private function _getUpload($fileId)
    {

        //Prevent from being loaded twice
        if ($this->_uploadId > 0) {
            return;
        }

        $query = "SELECT a.*,COUNT(b.upload_id) AS parts_uploaded FROM backup_upload AS a LEFT JOIN backup_upload_part AS b ON a.upload_id=b.upload_id WHERE a.file_id=UNHEX(?)";
        if ($stmt = mysqli_prepare($this->_dbconn, $query)) {
            $stmt->bind_param('s', $fileId);
            if ($stmt->execute()) {
                if ($result = $stmt->get_result()) {

                    $this->_uploadRow = $result->fetch_assoc();
										$this->_uploadRow['hash'] = bin2hex($this->_uploadRow['hash']);

                    //Set internal member vars
                    $this->_uploadId = $this->_uploadRow['upload_id'];
                    $this->_fileId = bin2hex($this->_uploadRow['file_id']);
                    $this->_totalParts = (int)$this->_uploadRow['parts'];
                    $this->_partsUploaded = (int)$this->_uploadRow['parts_uploaded'];
                    $this->_partsRemaining = $this->_totalParts - $this->_partsUploaded;

                }
								else {
                    throw new APIException("The upload record could not be found. (" . mysqli_error($this->_dbconn) . ")");
                }
            }
						else {
                throw new APIException("There was an error querying for the upload. (" . mysqli_error($this->_dbconn) . ")");
            }

            $stmt->close();
        }
				else {
            throw new APIException("There was an error querying for the upload. (" . mysqli_error($this->_dbconn) . ")");
        }
    }

    public function getUpload()
    {
        return $this->_uploadRow;
    }

    private function _isIgnoreType($fileType)
    {
        $ignore=false;

        $query = "SELECT type_id FROM backup_ignore_type WHERE file_type=? AND active=1";

        if ($stmt = mysqli_prepare($this->_dbconn, $query)) {
            $stmt->bind_param('s', $fileType);
            $stmt->execute();
            if ($stmt->get_result()->num_rows > 0) {
                $ignore=true;
            }

            $stmt->close();
        }

        return $ignore;
    }

    /* Initialize a new file upload and return the upload Id to the client */
    public function initUpload()
    {
        $requiredFields = array(
            'file_name',
            'file_size',
            'file_type',
            'hash',
            'file_path',
            'parts',
            'last_modified',
            'compressed'
        );

        //Verify required fields exist
        foreach ($requiredFields as $key => $val) {
            $fv = $this->_metadata->{$val};

            if (empty($fv) && gettype($fv) != "boolean") {
                throw new APIException("Required field is missing: " . $val);
            }
        }

        //Verify minimum file size for multi part upload
        if ($this->_metadata->{'parts'} > 1) {
            $minimumFilesize = (int)$this->_db->getClientSetting('multipart_filesize');
            if ($this->_metadata->{'file_size'} < $minimumFilesize) {
                throw new APIException("File does not meet the minimum filesize for a multi-part upload: " . $minimumFilesize);
            }
        }

        //Check if filetype should be ignored
        $fileType = strtolower($this->_metadata->{'file_type'});
        if ($this->_isIgnoreType($fileType)) {
            throw new APIException("File type " . $fileType . " is not allowed to upload");
        }

        //The database file ID is the SHA-1 hash of the complete (canonical) file path
        $fileId = sha1($this->_metadata->{'file_path'} . "/" . $this->_metadata->{'file_name'});

        //Check and see if the file already exists in the database
        //If the hash is unchanged AND it exists, cancel the upload
        $tmpFile = new BackupFile($fileId, $this->_userId);
        if ($tmpFile->exists()) {
            if ( $tmpFile->getValue('hash') == $this->_metadata->{'hash'} && ($tmpFile->getValue('uploaded') == 1) ) {
                throw new APIException("File has already been uploaded");
            }
        }

        //Should we compress the file and store it?
        $storeCompressed = (bool)$this->_db->getSetting('store_compressed');

        //Set target filename
        $targetFileName = $storeCompressed ? ($this->_metadata->{'file_name'} . ".gz") : $this->_metadata->{'file_name'};

        //Insert a new file
        $query = "REPLACE INTO backup_file (file_id,file_name,file_size,user_id,file_type,hash,file_path,compressed,last_modified,last_backup) VALUES(UNHEX(?),?,?,?,?,UNHEX(?),?,?,FROM_UNIXTIME(?),NOW())";
        if ($stmt = mysqli_prepare($this->_dbconn, $query)) {
            $stmt->bind_param(
                'ssiisssii',
                $fileId,
                $targetFileName,
                $this->_metadata->{'file_size'},
                $this->_userId,
                $this->_metadata->{'file_type'},
                $this->_metadata->{'hash'},
                $this->_metadata->{'file_path'},
                $storeCompressed,
                $this->_metadata->{'last_modified'}
            );

            if ($stmt->execute()) {
                $this->_fileId = $fileId;
            } else {
                throw new APIException("There was an error inserting the file. (" . mysqli_error($this->_dbconn) . ")");
            }

            $stmt->close();
        }

        if ( empty($this->_fileId) ) {
            throw new APIException("There was an error inserting the file. (" . mysqli_error($this->_dbconn) . ")");
        }

        //Create new upload and associate it with the file
        $query = "INSERT INTO backup_upload (file_id,user_id,parts,bytes,hash,compressed) VALUES(UNHEX(?),?,?,?,UNHEX(?),?)";
        if ($stmt = mysqli_prepare($this->_dbconn, $query)) {
            $stmt->bind_param(
                'siiisi',
                $this->_fileId,
                $this->_userId,
                $this->_metadata->{'parts'},
                $this->_metadata->{'file_size'},
                $this->_metadata->{'hash'},
                $storeCompressed
            );

            if ($stmt->execute()) {
                $this->_uploadId = mysqli_insert_id($this->_dbconn);
            }

            $stmt->close();
        }

        if ($this->_uploadId < 0) {
            throw new APIException("There was an error creating the file upload (" . mysqli_error($this->_dbconn) . ")");
        }

        return $this->_uploadId;
    }

    /**
    * Upload file contents
    */
    public function uploadPart($contents)
    {
        if (empty($contents)) {
            throw new APIException("File contents are empty");
        }

        $requiredFields = array(
            'file_id',
            'part_number',
            'part_size',
            'hash',
            'compressed'
        );

        //Verify required fields exist
        foreach ($requiredFields as $key => $val) {
            $fv = $this->_metadata->{$val};

            if (empty($fv) && gettype($fv) != "boolean") {
                throw new APIException("Required field is missing: " . $val);
            }
        }

        //Verify Upload Exists
        if (!$this->_uploadRow) {
            throw new APIException("There is no pending upload for file id " . $this->_metadata->{'file_id'} . " does not exist");
        }

        //Verify SHA-1 hash
        $contentHash = sha1($contents);

        if ( $contentHash != strtolower($this->_metadata->{'hash'}) ) {
            throw new APIException("File contents integrity check failed. SHA-1 hash mismatch- the file contents are not correct.");
        }

        //Verify part size
        $contentSize = strlen($contents);
        if ($this->_metadata->{'part_size'} != $contentSize) {
            throw new APIException("File part error: Total byte size is not correct.");
        }

        //Is file content compressed
        $compressed = $this->_metadata->{'compressed'};

        //When the file was initialized, was it flagged as compressed
        $storeCompressed = (bool)$this->_file->getValue('compressed');

        /**
         ** Set class file contents
        **/

        //Determine whether we need to compress or decompress content
        if ($storeCompressed && !$compressed) {
            $this->_contentBytes = gzdeflate($contents);
        } elseif (!$storeCompressed && $compressed) {
            $this->_contentBytes = gzinflate($contents);
        } else { //No action needed
            $this->_contentBytes = $contents;
        }

        //Get the associated backup file
        if (!$this->_file) {
            throw new APIException("File with id " . $this->_fileId . " does not exist");
        }

        //Check for valid part number
        if ($this->_metadata->{'part_number'} > $this->_uploadRow['parts']) {
            throw new APIException("Invalid part number was specified.");
        }

        if (!$this->_backupTarget) {
            throw new APIException("Could not find storage target");
        }

        //Write file content to storage
        //Storage Target Handlers here
        if ($this->_uploadRow['parts'] <= 1) {
            $fileName = $storeCompressed ? ($this->_file->getValue('file_name') . ".gz") : $this->_file->getValue('file_name');
        } else {
            $fileName = $this->_file->getValue('file_id') . "_" . $this->_metadata->{'part_number'} . ".part";
        }

        if ($this->_backupTarget['type'] == "local") {

            //If there is only one part, write to the file path. If there are multiple parts, we write to the parts folder
            $filePath = $this->_backupTarget['path'];

            if ($this->_uploadRow['parts'] <= 1) {
                $filePath .= "/users/" . $this->_userId . $this->_file->getValue('file_path');
            } else {
                $filePath .= "/parts";
            }

            $filePath .= "/" . $fileName;

						echo "Filepath is: " . $filePath . "\n";
						echo "File ID is: " . $this->_file->getValue('file_id') . "\n";
						flush();

            if (!$this->_writeLocalFile($filePath)) {
                throw new APIException("Failed to write file to local storage. Please check the path and folder permissions (" . $this->getError() . ")");
            }
        }

        //Add a new part to the database if the write operation was successful
        $partId = -1;
        $query = "REPLACE INTO backup_upload_part (upload_id,part_number,bytes,tmp_file_name,hash,compressed) VALUES(?,?,?,?,UNHEX(?),?)";
        if ($stmt = mysqli_prepare($this->_dbconn, $query)) {
            $stmt->bind_param(
                'iiissi',
                $this->_uploadId,
                $this->_metadata->{'part_number'},
                $contentSize,
                $fileName,
                $contentHash,
                $storeCompressed
            );

            if ($stmt->execute()) {
                $partId = mysqli_insert_id($this->_dbconn);
            }

            $stmt->close();
        }

        if ($partId < 0) {
            throw new APIException("Failed to add upload part to the database (" . mysqli_error($this->_dbconn) . ")");
        }

        //Subtract one from partsRemaining for newly uploaded part
        $this->_partsRemaining = ($this->_totalParts - (++$this->_partsUploaded));

        //If all parts have been uploaded, complete the upload
        if ($this->_partsRemaining <= 0) {
            $this->completeUpload();
        }

        return $partId;
    }

    private function _getPartCount($uploadId)
    {
        $total=0;

        $query = "SELECT COUNT(*) FROM backup_upload_part WHERE upload_id=" . $uploadId;
        if ($result = mysqli_query($this->_dbconn, $query)) {
            $total = $result[0];

            $result->close();
        } else {
            throw new APIException("Failed to retrieve upload parts (" . mysqli_error($this->_dbconn) . ")");
        }

        return $total;
    }

    /**
     ** Complete the file upload
     ** Set the uploaded flag to "1"
     ** Merge upload parts together if all have been completed and more than one
    */
    public function completeUpload($uploadId=-1)
    {

        //If an upload ID is passed, load a new upload row
        //If an id was not passed, use the constructed upload row
        if ($uploadId > 0 && !$this->_uploadRow) {
            $this->_getUpload($uploadId);
            if ( !$this->_file = new BackupFile($this->_fileId, $this->_userId) ) {
							throw new APIException('File id does not exist: ' . $this->_fileId);
						}
        }

        /** File is more than one part
         ** Check if all parts have been uploaded, if so, merge into one file and save into destination directory
        **/

        $result = null;
        $query = "SELECT * FROM backup_upload_part WHERE upload_id=? ORDER BY part_number ASC";
        if ($stmt = mysqli_prepare($this->_dbconn, $query)) {
            $stmt->bind_param('i', $this->_uploadId);

            if ($stmt->execute()) {
                $result = $stmt->get_result();

                $totalParts = $result->num_rows;

                //Ensure that all parts have been uploaded
                if ($totalParts < $this->_uploadRow['parts']) {
                    throw new APIException("Failed to complete upload. All parts have not been uploaded yet.");
                }

                //Merge the file parts together into one file if more than one part
                if ($totalParts > 1) {
                    if ($this->_backupTarget['type'] == "local") {
                        $filePath = $this->_backupTarget['path'] . "/users/" . $this->_userId . "/" . $this->_file->getValue('file_path') . "/" . $this->_file->getValue('file_name');

                        //Prepare directory
                        if (!$this->_mkDirIfNotExists(dirname($filePath))) {
                            throw new APIException("Could not create directory: " . $this->getError());
                        }

                        //Create the file for writing
                        if ($writeFile = @fopen($filePath, 'w')) {

                            //Get all tmp file parts and merge into one file
                            while ($row = $result->fetch_assoc()) {
                                $fp = $this->_backupTarget['path'] . "/parts/" . $row['tmp_file_name'];

                                if ($tmpFile = @fopen($fp, 'r')) {
                                    if ($bytes = @fread($tmpFile, filesize($fp))) {

                                        //Write bytes to the target file
                                        if (!fwrite($writeFile, $bytes)) {
                                            throw new APIException("Failed to write content to target file: " . $filePath);
                                        }
                                    } else {
                                        throw new APIException("Unable to read file part: " . $fp);
                                    }

                                    @fclose($tmpFile);
                                } else {
                                    throw new APIException("Unable to open file for writing: " . $fp);
                                }
                            }

                            @fclose($writeFile);
                        }

                        //Cleanup and delete upload parts after all operations were previously successful
                        if ($result) {

                            //Reset pointer to 1st index
                            $result->data_seek(0);

                            while ($row = $result->fetch_assoc()) {
                                $fp = $this->_backupTarget['path'] . "/parts/" . $row['tmp_file_name'];

																try {
	                                if (!unlink($fp)) {
	                                    throw new APIException("Unable to delete file part: " . $fp); //Non-fatal error
	                                }
																}
																catch( APIException $ex ) {

																}
                            }

                            //Cleanup DB
														try {
	                            if (!mysqli_query($this->_dbconn, "DELETE a,b FROM backup_upload AS a INNER JOIN backup_upload_part AS b ON a.upload_id=b.upload_id WHERE a.upload_id=" . $this->_uploadId)) {
	                                throw new APIException("Failed to cleanup upload parts: " . $this->_uploadId . " (" . mysqli_error($this->_dbconn) . ")"); //Non-fatal error
	                            }
														}
														catch ( APIException $ex ) {

														}

                            //Mark completed after all parts have been written
                            if ($this->_setFileUploaded($this->_uploadRow['file_id'])) {
                                $this->_uploadComplete=true;
                            }
                        }
                    }
                }
            }

            $stmt->close();
        } else {
            throw new APIException("There was an error retrieving the upload part(s): " . mysqli_error($this->_dbconn));
        }

        return true;
    }

    private function _mkDirIfNotExists($path)
    {

        //Check if directory exists, if not then create it
        if (!is_dir($path)) {
            if (mkdir($path, 0777, true)) {
                chmod($path, 0777);
            } else {
                throw new APIException("Failed to create directory or set directory permissions: " . $path);
            }
        }

        //Try to CHMOD the dir if not writable before failing completely
        if (!is_writable($path)) {
            if (chmod($path, 0777)) {
                return true;
            }
            return false;
        }

        return true;
    }

    /**
     ** Sets the backup_file "uploaded" column to 1
    **/
    private function _setFileUploaded($fileId)
    {
        $query = "UPDATE backup_file SET uploaded=1 WHERE file_id=UNHEX(?) AND user_id=?";
        if ($stmt = mysqli_prepare($this->_dbconn, $query)) {
            $stmt->bind_param('si', $fileId, $this->_userId);
            if (!$stmt->execute()) {
                throw new APIException("Failed to set file to uploaded=1 (" . mysqli_error($this->_dbconn) . ")");
            }

            $stmt->close();
        }

        return true;
    }

    private function _writeLocalFile($filePath)
    {
        $base_dir = dirname($filePath);

        //Check if directory exists, if not then create it
        if (!is_dir($base_dir)) {
            if (mkdir($base_dir, 0777, true)) {
                chmod($base_dir, 0777);
            } else {
                throw new APIException("Failed to create directory or set directory permissions: " . $base_dir);
            }
        }

        //Try to CHMOD the dir if not writable before failing completely
        if (!is_writable($base_dir)) {
            chmod($base_dir, 0777);
        }

        //Save file
        if (is_writable($base_dir)) {
            if ($fh = @fopen($filePath, 'w')) {
                if (!@fwrite($fh, $this->_contentBytes)) {
                    throw new APIException("Could not write to file " . $filePath);
                }

                fclose($fh);
            } else {
                throw new APIException("The file handle could not be obtained: " . $filePath);
            }
        } else {
            throw new APIException("The file is not writable: " . $filePath);
        }

        return true;
    }

    private function _getTarget()
    {
        $backupTarget = null;
        $hasUserTarget = false;

        //Check if there are any individual user targets configured
        $query = "SELECT a.* FROM backup_target AS a INNER JOIN backup_user_target AS b ON a.target_id=b.target_id WHERE b.user_id=" . $this->_userId;
        if ($result = mysqli_query($this->_dbconn, $query)) {
            if ($backupTarget = mysqli_fetch_array($result)) {
                $hasUserTarget=true;
            }

            $result->close();
        }

        if ($hasUserTarget) {
            return $backupTarget;
        }

        //Find the default Global target
        $query = "SELECT a.* FROM backup_target AS a INNER JOIN backup_setting AS b ON CAST(a.target_id AS CHAR(1)) = b.value WHERE b.name = 'default_target'";
        if ($result = mysqli_query($this->_dbconn, $query)) {
            if ($backupTarget = mysqli_fetch_array($result)) {
                $hasUserTarget=false;
            }

            $result->close();
        }

        return $backupTarget;
    }
}
