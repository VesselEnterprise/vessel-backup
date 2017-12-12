<?php

require_once 'API.class.php';

class SecureAPI extends API
{
	
	private $_origin = '';
	
	public function __construct($request) {
		
		parent::__construct($request);
		
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