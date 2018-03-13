<?php

require_once 'core/common.inc.php';

$renderer = new PageRenderer();
$renderer->addTemplate('header');

//Display registration form
if ( !isset($_POST['action']) ) {

  $renderer->addTemplate('register', array('refererUrl' => $common['session']->getRefererURL()) );

}

if ( isset($_POST['action']) && $_POST['action'] == "register") {

  if ( !isset($_POST['user_name']) || $_POST['user_name'] == "" ) {
    die("Error: No username was provided");
  }
  else if ( !isset($_POST['password']) || $_POST['password'] == "" ) {
    die("Error: No password was provided");
  }

  if ( strlen($_POST['user_name']) < 3 ){
    die("Username must be at least 3 chars");
  }

  if ( strlen($_POST['password']) < 7 ) {
    die("Password must be at least 7 characters");
  }

  $username = $_POST['user_name'];
  $password = password_hash($_POST['password'], PASSWORD_DEFAULT );
  $email = $_POST['email'];
  $first_name = $_POST['first_name'];
  $last_name = $_POST['last_name'];
  $title = $_POST['title'];
  $office = $_POST['office'];
  $address = $_POST['address'];
  $city = $_POST['city'];
  $state = $_POST['state'];
  $zip = $_POST['zip'];

  //Check if the user already exists
  if ( BackupUser::userExists($username) ) {
    die("Error: This username already exists");
  }

  //Insert a new user
  $dbconn = BackupDatabase::getDatabase()->getConnection();

  $query = "INSERT INTO backup_user (user_name,first_name,last_name,email,address,city,state,zip,title,office,password,password_set) VALUES(?,?,?,?,?,?,?,?,?,?,?,NOW())";

  if ( $stmt = mysqli_prepare($dbconn,$query) ) {

    $stmt->bind_param(
      "sssssssssss",
      $username,
      $first_name,
      $last_name,
      $email,
      $address,
      $city,
      $state,
      $zip,
      $title,
      $office,
      $password
    );

    if ( $stmt->execute() ) {

      //Add new user activation
      $userID = mysqli_insert_id($dbconn);

      $user = new BackupUser($userID);
      $user->createUserActivation();

      BackupLog::getLog($userID)->addMessage("User " . $username . " has been registered", "User Registration");

      $renderer->addTemplate('generic_message', array("message" => "You have successfully been registered. After your account has been approved, you will receive an email confirmation to activate your account"));

    }

    $stmt->close();

  }

}


$renderer->addTemplate('footer');
$renderer->render();

?>
