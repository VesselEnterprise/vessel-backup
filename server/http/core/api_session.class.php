<?php

require_once 'database.class.php';

class BackupAPISession
{
	
	private static $factory;
	private $_db;
	private $_userId = -1;
	private $_errorMsg;
	private $_tokenExpired=false;
	private $_tokenExists=false;
	private $_isAuthenticated=false;
	
	public function __construct() {
		
		$this->_db = BackupDatabase::getDatabase();
		
		//Authenticate the client when the object is instantiated
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
		$query = "SELECT user_id,UNIX_TIMESTAMP(token_expiry) FROM backup_user WHERE access_token=?";
		if ( $stmt = mysqli_prepare($this->_db->getConnection(), $query ) ) {
			
			$stmt->bind_param('s', $accessTokenHashed );
			if ( $stmt->execute() ) {
				
				if ( $result = $stmt->get_result() ) {
					if ( $row = $result->fetch_row() ) {
						
						$this->_tokenExists=true;
						
						//Check if token is expired
						if ( time() > $row[1] ) {
							$this->_setError("Token is expired");
							$this->_tokenExpired=true;
						}
						else {
							$this->_userId = $row[0];
							$this->_isAuthenticated = true;
						}
						
					}
					else {
						$this->_setError("Token is invalid");	
					}
				}
				
			}
			
			$stmt->close();
			
		}
	
		
	}
							
	private function _setError($msg) {
		$this->_errorMsg = $msg;	
	}
	
	public function isTokenExpired() {
		return $this->_tokenExpired;	
	}
	
	public function logSession($endpoint, $http_method) {
		
		$ip = $_SERVER['REMOTE_ADDR'];
		
		$query = "INSERT INTO backup_api_session (user_id,endpoint,http_method,ip_address) VALUES(?,?,?,?)";
		if ( $stmt = mysqli_prepare($this->_db->getConnection(), $query) ) {
			
			$stmt->bind_param('isss', $this->_userId, $endpoint, $http_method, $ip);
			$stmt->execute();
			$stmt->close();
			
		}
		
	}
	
	public function isAuthenticated() {
		return $this->_isAuthenticated;
	}
	
	public function getUserId() {
		return $this->_userId;
	}
	
}

?>