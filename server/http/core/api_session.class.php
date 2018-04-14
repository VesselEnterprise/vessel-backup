<?php

require_once 'database.class.php';
require_once 'api_exception.class.php';

class BackupAPISession
{

	private static $factory;
	private $_db;
	private $_dbconn;
	private $_userId = null;
	private $_errorMsg;
	private $_tokenExpired=false;
	private $_tokenMatches=false;
	private $_isAuthenticated=false;

	public function __construct() {

		$this->_db = BackupDatabase::getDatabase();
		$this->_dbconn = $this->_db->getConnection();

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

		//Decode authorization header
		$headerDecoded = base64_decode($headers['Authorization']);

		//Get user ID and access token
		$authParts = explode(":", $headerDecoded, 2);

		if ( count($authParts) < 2 ) {
			throw new APIException("Authorization header is invalid");
		}

		$this->_userId = $authParts[0];
		$accessToken = $authParts[1];

		//Validate access token
		$query = "SELECT access_token,UNIX_TIMESTAMP(token_expiry) FROM backup_user WHERE user_id=UNHEX(?)";
		if ( $stmt = mysqli_prepare($this->_dbconn, $query ) ) {

			$stmt->bind_param('s', $this->_userId );
			if ( $stmt->execute() ) {

				if ( $result = $stmt->get_result() ) {
					if ( $row = $result->fetch_row() ) {

						//Check if the provided token matches the DB
						if ( password_verify($accessToken, $row[0]) == TRUE ) {
							$this->_tokenMatches=true;
						}

						//Check if token is expired
						if ( time() > $row[1] ) {
							$this->_setError("Token is expired");
							$this->_tokenExpired=true;
						}

						//If the token matches and it's not expired, user is authenticated
						if ( $this->_tokenMatches && !$this->_tokenExpired ) {
							$this->_isAuthenticated=true;
						}

					}
					else {
						$this->_setError("Invalid User Id was detected");
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

		$query = "INSERT INTO backup_api_session (user_id,endpoint,http_method,ip_address) VALUES(UNHEX(?),?,?,?)";
		if ( $stmt = mysqli_prepare($this->_db->getConnection(), $query) ) {

			$stmt->bind_param('ssss', $this->_userId, $endpoint, $http_method, $ip);
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
