<?php

ini_set('display_errors', 1);
error_reporting(E_ALL);

require_once '../../core/common.inc.php';
require_once '../../core/backup_api.class.php';

$conn = BackupDatabase::getDatabase()->getConnection();

$api = new BackupAPI( $_REQUEST['request'] );
$api->processAPI();

?>