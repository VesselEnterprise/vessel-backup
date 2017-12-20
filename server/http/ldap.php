<?php

require_once 'core/common.inc.php';
require_once 'core/ldap.class.php';

if ( !$common['session']->isLoggedIn() ) {
	$common['session']->printLoginMsg();
	return;
}

if ( empty($_GET['action']) && empty($_POST['action']) ) {
	
	echo "<ul>";
	echo "<li><a href=\"" . $_SERVER['PHP_SELF'] . "?action=user_import\">Import LDAP Users</a></li>";
	echo "<li><a href=\"" . $_SERVER['PHP_SELF'] . "?action=computer_import\">Import LDAP Computers</a></li>";
	echo "</ul>";	
	
	return;
	
}

if ( $_GET['action'] == "user_import" ) {

	$ldap = new BackupLDAP();
	$ldap->importUsers();
	
	return;
	
}

if ( $_GET['action'] == "computer_import" ) {
	
	$ldap = new BackupLDAP();
	$ldap->importComputers();
	
	return;
	
}

?>