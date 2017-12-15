<?php

class BackupDatabase
{
	
	private static $factory; //For connections across multiple objects
	private $dbConn = Null;
	
	/*
	Enter MySQL / MariaDB details here
	*/
	private $dbUser = 'root';
	private $dbPwd = 'root';
	private $dbName = 'backup';
	private $dbServer = 'localhost';
	private $dbPrefix = 'backup_';
	private $dbType = 'mysql';
	
	public function __construct() {
		
		$this->connect();
		
	}
	
	public function __destruct() {
		
		$this->disconnect();
		
	}
	
	private function connect() {
		
		$this->dbConn = mysqli_connect( $this->dbServer, $this->dbUser, $this->dbPwd, $this->dbName );
		if ( !$this->dbConn ) {
			die( 'Could not connect: ' . mysqli_error() );
		}
		//echo 'Connected successfully';

	}
	
	public function disconnect() {
		
		mysqli_close($this->dbConn);
		
	}
	
	public static function getDatabase()
	{
		if (!self::$factory)
			self::$factory = new BackupDatabase();
		
		return self::$factory;
	}

	public function getConnection() {
		
		if ( !$this->dbConn )
			$this->connect();
		
		return $this->dbConn;
	}
	
	public function getSetting($name) {
		
		$val = '';
		
		if ( $result = mysqli_query($this->dbConn, "SELECT value FROM backup_setting WHERE name='" . $name . "'") ) {
			
			if ( $row = $result->fetch_row() ) {
				$val = $row[0];
			}
			
			$result->close();
			
		}
		else {
		
			echo "Error: Could not retrieve setting (" . mysqli_error($this->_dbConn) . ")";
		
		}
		
		return $val;
		
	}

}

?>