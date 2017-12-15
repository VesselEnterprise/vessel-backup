<?php

require_once 'core/common.inc.php';
require_once 'core/ldap.class.php';

if ( $_GET['action'] == "user_import" ) {

	$ldap = new BackupLDAP();
	$ldap->importUsers();
	
}

if ( $_GET['action'] == "computer_import" ) {
	
	$ldap = new BackupLDAP();
	$ldap->importComputers();
	
}

?>