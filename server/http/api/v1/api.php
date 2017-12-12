<?php

ini_set('display_errors', 1);
error_reporting(E_ALL);

spl_autoload_register(function ($class_name) {
    require_once $class_name . '.class.php';
});

$conn = Database::getFactory()->getConnection();

$api = new SecureAPI( $_REQUEST['request'] );
$api->processAPI();

echo $globals;

?>