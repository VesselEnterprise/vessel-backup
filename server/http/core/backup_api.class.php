<?php

require_once 'database.class.php';
require_once 'api.class.php';

class BackupAPI extends API
{
	
	private $_db;
	private $_origin = '';
	private $_authenticated = false;
	
	public function __construct($request) {
		
		parent::__construct($request);
		
		$this->_db = BackupDatabase::getDatabase();
		
		$this->_setOrigin();
		
		$this->_authenticate();
	
	}
	
	private function _setOrigin() {
		
		if (array_key_exists('HTTP_ORIGIN', $_SERVER)) {
			$_origin = $_SERVER['HTTP_ORIGIN'];
		}
		else if (array_key_exists('HTTP_REFERER', $_SERVER)) {
			$_origin = $_SERVER['HTTP_REFERER'];
		} else {
			$_origin = $_SERVER['REMOTE_ADDR'];
		}
		
	}
	
	private function _authenticate() {
		
		//Reset
		$this->_authenticated = false;
		
		//Get Authorization Header
		$headers = apache_request_headers();
		
		if ( !isset($headers['Authorization'] ) ) {
			$this->_authenticated = false;
			return;
		}
		
		$access_token = $headers['Authorization'];
		
		//Validate access token
		$query = "SELECT user_id FROM backup_user WHERE access_token=?";
		if ( $stmt = mysqli_prepare($this->_db->getConnection(), $query ) ) {
			
			$stmt->bind_param('s', $access_token );
			if ( $stmt->execute() ) {
				
				$result = $stmt->get_result();
				
				if ( $result->num_rows > 0 )
					$this->_authenticated = true;
				
			}
			
			$stmt->close();
			
		}
		
	}
	
	public function isAuthenticated() {
		return $this->_authenticated;
	}
	
}

?>