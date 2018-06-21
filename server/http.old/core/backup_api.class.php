<?php

require_once 'database.class.php';
require_once 'api.class.php';

class BackupAPI extends API
{
	
	private $_db;
	private $_origin = '';
	
	public function __construct($request) {
		
		parent::__construct($request);
		
		$this->_db = BackupDatabase::getDatabase();
		
		$this->_setOrigin();
	
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
	
}

?>