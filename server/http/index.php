<?php

ini_set('display_errors', 1);
error_reporting(E_ALL);

require_once './core/ldap.class.php';

$ldap = new ldap("brightview.net", 389, "svc_SN_LDAP", "Wm8kKm262j");

$ldap->importUsers()

?>