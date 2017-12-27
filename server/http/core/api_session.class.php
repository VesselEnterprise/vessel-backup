<?php

require_once 'database.class.php';

class BackupAPISession
{
	
	private static $factory;
	private $_db;
	private $_userID = -1;
	private $_isAuthenticated = false;
	
	public function __construct() {
		
		$this->_db = BackupDatabase::getDatabase();
		
		//Authenticate the client
		$this->_authenticate();
		
	}
	
	public static function getSession()
	{
		if (!self::$factory)
			self::$factory = new BackupAPISession();
		
		return self::$factory;
	}
	
	/**
	* Checks the Authorization header for a valid access token
	*/
	private function _authenticate() {
		
		//Reset
		$this->_isAuthenticated = false;
		
		//Get Authorization Header
		$headers = apache_request_headers();
		
		if ( !isset($headers['Authorization'] ) ) {
			$this->_isAuthenticated = false;
			return;
		}
		
		$accessTokenHashed = sha1( $headers['Authorization'] );
		
		//Validate access token
		$query = "SELECT user_id FROM backup_user WHERE access_token=?";
		if ( $stmt = mysqli_prepare($this->_db->getConnection(), $query ) ) {
			
			$stmt->bind_param('s', $accessTokenHashed );
			if ( $stmt->execute() ) {
				
				$result = $stmt->get_result();
				
				if ( $result->num_rows > 0 ) {
					$this->_userID = $result->fetch_row()[0];
					$this->_isAuthenticated = true;
				}
				
			}
			
			$stmt->close();
			
		}
	
		
	}
	
	public function logSession($endpoint, $http_method) {
		
		$ip = $_SERVER['REMOTE_ADDR'];
		
		$query = "INSERT INTO backup_api_session (user_id,endpoint,http_method,ip_address) VALUES(?,?,?,?)";
		if ( $stmt = mysqli_prepare($this->_db->getConnection(), $query) ) {
			
			$stmt->bind_param('isss', $this->_userID, $endpoint, $http_method, $ip);
			$stmt->execute();
			$stmt->close();
			
		}
		
	}
	
	public function isAuthenticated() {
		return $this->_isAuthenticated;
	}
	
	public function getUserID() {
		return $this->_userID;
	}
	
}

?>