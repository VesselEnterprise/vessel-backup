<?php

public class Database {
	
	private $dbUser = 'root';
	private $dbpass = 'root';
	private $dbName = 'backup';
	private $dbServer = 'localhost';
	private $dbPrefix = 'backup_';
	private $dbType = 'mysql';
	
	protected $dbConn = Null;
	
	public function __construct() {
		
		$this->connect();
		
	}
	
	public function __destruct() {
		
		$this->disconnect();
		
	}
	
	private function connect() {
		
		$this->dbConn = mysqli_connect( $this->dbServer, $this->dbUser, $this->dbPass, $this->dbName );
		if ( !$this->dbConn ) {
			die( 'Could not connect: ' . mysqli_error() );
		}
		echo 'Connected successfully';

	}
	
	public function disconnect() {
		
		mysql_close($this->dbConn);
		
	}

}

?>