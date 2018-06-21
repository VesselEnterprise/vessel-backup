<?php

require_once 'database.class.php';

class BackupUser
{

	private static $factory;

	private $_userId=null;
	private $_db;
	private $_dbconn;
	private $_log;
	private $_userRow;
	private $_exists = false;

	public function __construct($userId) {

		//Get database connection
		$this->_db = BackupDatabase::getDatabase();
		$this->_dbconn = $this->_db->getConnection();

		//Get Log Object
		$this->_log = BackupLog::getLog($userId);

		$this->_userRow = $this->_getUserRow($userId);

	}

	function __destruct() {

	}

	private function _getUserRow($userId) {

		$row = array();

		$query = "SELECT * FROM backup_user WHERE user_id=UNHEX(?) OR user_name=?";
		if ( $stmt = mysqli_prepare($this->_dbconn, $query ) ) {
			$stmt->bind_param('ss', $userId, $userId );
			if ( $stmt->execute() ) {
				if ( $result = $stmt->get_result() ) {
					$row = $result->fetch_assoc();
					$this->_userId = bin2hex($row['user_id']);
					$row['user_id'] = $this->_userId;
					$this->_exists=true;
				}
			}
			$stmt->close();
		}

		return $row;

	}

	public function getValue($col) {
		return $this->_userRow[$col];
	}

	public function getUserId() {
		return $this->_userId;
	}

	public function exists() {
		return $this->_exists;
	}

	public static function getUser($userId)
	{
		if (!self::$factory)
			self::$factory = new BackupUser($userId);

		return self::$factory;
	}

	public function setPassword($pwd) {

		$hash = password_hash( $pwd, PASSWORD_DEFAULT );

		//Update user password
		if ( $stmt = mysqli_prepare("UPDATE backup_user SET password=?,password_set=NOW() WHERE user_id=UNHEX(?)") ) {

			$stmt->bind_param('ss', $hash, $this->_userId );
			$stmt->execute();
			$stmt->close();

			$this->_log->addMessage("User (User Id = " . $this->_userId . ") changed their password");

		}

	}

	public static function generateAccessToken() {
		return substr(bin2hex(random_bytes(32)),0,32);
	}

	public function createUserActivation($code='') {

		$status=false;

		if ( !$code )
			$code = $this->generateAccessToken();

		$query = "INSERT INTO backup_user_activation (user_id,code,expiry) VALUES(UNHEX(?),?,(NOW() + INTERVAL 7 DAY))";

		if ( $stmt = mysqli_prepare($this->_dbconn, $query ) ) {

			$stmt->bind_param('ss', $this->_userId, $code );
			if ( $stmt->execute() ) {
				$status=true;
			}
			else
				echo "Error: " . mysqli_error($this->_dbconn);

			$stmt->close();

		}

		return $status;

	}

	public function addRefreshToken() {

		$refreshToken = $this->generateAccessToken();
		$refreshTokenHashed = password_hash($refreshToken, PASSWORD_DEFAULT);

		$query = "REPLACE INTO backup_refresh_token (user_id,refresh_token) VALUES(UNHEX(?),?)";
		if ( $stmt = mysqli_prepare($this->_dbconn, $query) ) {

			$stmt->bind_param('ss', $this->_userId, $refreshTokenHashed);
			if ( !$stmt->execute() ) {
				$this->_log->addError("Failed to add refresh token (user_id=" . $this->_userId . ") (" . mysqli_error($this->_dbconn) . ")", "Authentication");
			}

			$stmt->close();

		}

		return $refreshToken;

	}

	public function activateUser($activationCode) {

		$accessToken = '';

		$query = "SELECT user_id FROM backup_user_activation WHERE user_id=UNHEX(?) AND code=? AND expiry >= NOW()";

		if ( $stmt = mysqli_prepare($this->_dbconn, $query ) ) {

			$stmt->bind_param('ss', $this->_userId, $activationCode );

			if ( $stmt->execute() ) {

				$result = $stmt->get_result();
				if ( $result->num_rows > 0 ) {

					//User is now activated
					$accessToken = $this->generateAccessToken();
					$accessTokenHashed = password_hash( $accessToken, PASSWORD_DEFAULT );

					//Set the user's access key
					if ( $s2 = mysqli_prepare($this->_dbconn, "UPDATE backup_user SET access_token=?,token_expiry=(NOW()+INTERVAL ? HOUR),activated=1 WHERE user_id=UNHEX(?)") ) {

						//Get token lifetime
						$tokenExpiry = (int)$this->_db->getSetting('token_expiry');

						$s2->bind_param('sis', $accessTokenHashed, $tokenExpiry, $this->_userId);
						if ( $s2->execute() ) {
							$this->_log->addMessage("Successfully activated user (user_id=" . $this->_userId . ")", "User Activation");
						}

						$s2->close();

						//Remove the user activation record
						@mysqli_query($this->_dbconn, "DELETE FROM backup_user_activation WHERE user_id=UNHEX('" . $this->_userId . "')" );
					}
					else {
						$this->_log->addError("Error: Failed to activate user (user_id=" . $this->_userId . ") (" . mysqli_error($this->_dbconn) . ")", "User Activation");
					}

				}
				else {
					$this->_log->addError("Error: Failed to activate user (user_id=" . $this->_userId . ") (" . mysqli_error($this->_dbconn) . ")", "User Activation");
				}

			}
			else {
				$this->_log->addError("Error: Failed to activate user (user_id=" . $this->_userId . "(" . mysqli_error($this->_dbconn) . ")", "User Activation");
			}


			$stmt->close();

		}

		return $accessToken;

	}

	public function refreshAccessToken($refreshToken) {

		$accessToken = '';

		//Find the associated refresh token for the associated user
		$query = "SELECT refresh_token FROM backup_refresh_token WHERE user_id=UNHEX(?)";
		if ( $stmt = mysqli_prepare($this->_dbconn, $query) ) {

			$stmt->bind_param('s', $this->_userId);
			if ( $stmt->execute() ) {
				if ( $result = $stmt->get_result() ) {
					$row = $result->fetch_assoc();
					if ( password_verify($refreshToken, $row['refresh_token']) == TRUE ) {
						$accessToken = $this->generateAccessToken();
					}
				}
			}

			$stmt->close();

		}

		//Update the user row and Delete the existing refresh token if the validation was successful
		if ( !empty($accessToken) ) {

			$accessTokenHashed = password_hash($accessToken, PASSWORD_DEFAULT);

			$query = "UPDATE backup_user SET access_token=?,token_expiry=(NOW()+INTERVAL ? HOUR) WHERE user_id=UNHEX(?)";
			if ( $stmt = mysqli_prepare($this->_dbconn, $query) ) {

				//Get token lifetime
				$tokenExpiry = (int)$this->_db->getSetting('token_expiry');

				$stmt->bind_param('sis', $accessTokenHashed, $tokenExpiry, $this->_userId);
				if ( !$stmt->execute() ) {
					$this->_log->addError("Failed to update user access token (user_id=" . $this->_userId . "(" . mysqli_error($this->_dbconn) . ")", "Authentication");
				}

				$stmt->close();
			}

			//Delete the existing refresh token
			@mysqli_query($this->_dbconn, "DELETE FROM backup_refresh_token WHERE user_id=UNHEX('" . $this->_userId . "')");
		}

		return $accessToken;

	}

	public function getTokenExpiry() {

		$tokenExpiry=0;

		$query = "SELECT UNIX_TIMESTAMP(token_expiry) FROM backup_user WHERE user_id=UNHEX(?)";
		if ( $stmt = mysqli_prepare($this->_dbconn, $query) ) {

			$stmt->bind_param('s', $this->_userId);
			if ( $stmt->execute() ) {
				if ( $result = $stmt->get_result() ) {
					$row = $result->fetch_row();
					$tokenExpiry = $row[0];
				}
			}

			$stmt->close();
		}

		return $tokenExpiry;

	}

	public static function userExists($userName) {

		$exists=false;

		$query = "SELECT user_id FROM backup_user WHERE user_name = ?";
		if ( $stmt = mysqli_prepare(BackupDatabase::getDatabase()->getConnection(), $query) ) {

			$stmt->bind_param('s', $userName );
			if ( $stmt->execute() ) {
				$result = $stmt->get_result();
				if ( mysqli_num_rows($result) > 0 )
					$exists=true;
			}

			$stmt->close();

		}

		return $exists;

	}

}

?>
