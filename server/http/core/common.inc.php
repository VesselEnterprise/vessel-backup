<?php

/*
Common include file used to include important classes used in most scripts and pages
*/

ini_set('display_errors', 1);
error_reporting(E_ALL);

require_once 'session.class.php';
require_once 'database.class.php';
require_once 'log.class.php';
require_once 'user.class.php';

//Start a session
$common['session'] = BackupSession::getSession();


?>