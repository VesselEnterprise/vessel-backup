<?php

class ldap
{
	
	private $_lc; //LDAP connection handle
	private $_host;
	private $_port;
	private $_userdn;
	private $_pwd;
	
	public function __construct( $host, $port, $userdn, $pwd ) {
		
		$this->_host = $host;
		$this->_port = $port;
		$this->_userdn = $userdn;
		$this->_pwd = $pwd;
		
		if ( $this->_connect() ) {
			if ( !$this->_bind() ) {
				echo "Unable to bind to LDAP server";
				//handle error
			}
		}
		else {
			//Handle error
		}
		
	}
	
	function __destruct() {
		
		ldap_unbind($this->_lc);
		
		echo "Unbind successfully";
		
	}
	
	private function _connect() {

		$this->_lc = ldap_connect($this->_host, $this->_port);
		
		if ( !$this->_lc ) {
			echo "Unable to connect to LDAP server";
			//handle error
			return false;
		}
		
		ldap_set_option($this->_lc, LDAP_OPT_PROTOCOL_VERSION, 3); 
		ldap_set_option($this->_lc, LDAP_OPT_REFERRALS, 0); 
		
		echo "Connected successfully";
		
		return true;

	}
	
	private function _bind() {

		if ( !ldap_bind($this->_lc, $this->_userdn, $this->_pwd) )
			return false;
		
		return true;
	}
	
	public function importUsers() {
		
		if ( !$this->_lc ) {
			//Handle error
			return;
		}
		
		$dn = "OU=BV-Users,DC=brightview,DC=net";
		$filter = "(&(objectClass=person)(sn=*))";
		$attributes = array(
			"sAMAccountName", 
			"sn", 
			"givenname", 
			"mail", 
			"title", 
			"streetAddress", 
			"l", 
			"st", 
			"postalCode", 
			"physicalDeliveryOfficeName", 
			"mobile"
		);

		$users = ldap_search($this->_lc, $dn, $filter, $attributes, 0, 5000 );
		
		if ( !$users ) {
			//Handle error
			return;
		}

		$info = ldap_get_entries($this->_lc, $users);

		echo $info["count"]." entries returned\n";
		
	}
	
}

?>