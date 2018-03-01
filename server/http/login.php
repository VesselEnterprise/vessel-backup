<?php

require_once 'core/common.inc.php';

$renderer = new PageRenderer();
$renderer->addTemplate('header');

//Check if the user is already logged in
if ( $common['session']->isLoggedIn() ) {
  $renderer->addTemplate('generic_message', array("message" => "You are already logged in"));
}

  //Display login form
if ( !isset($_POST['action']) ) {

  $renderer->addTemplate('login_form', array('refererUrl' => $common['session']->getRefererURL()) );

}

if ( isset($_POST['action']) && $_POST['action'] == "login" ) {

	if ( !isset($_POST['user_name']) || $_POST['user_name'] == "" ) {
		die("Error: No username was provided");
	}
	else if ( !isset($_POST['password']) || $_POST['password'] == "" ) {
		die("Error: No password was provided");
	}

	$user_name = $_POST['user_name'];
	$password = $_POST['password'];

  //Attempt to login
	if ( $common['session']->login($user_name,$password) ) {

		echo "Login was successful!<br/>";

		if ( !empty($_POST['http_referer']) ) {

			echo "You will now be redirected in 3 seconds...";

			echo "<script>setTimeout(function() { window.location = \"" . $_POST['http_referer'] . "\"; }, 3000 );</script>";

		}
	}
	else {
		die("Error: Failed to authenticate. Please check your username or password.");
	}

}

$renderer->addTemplate('footer');
$renderer->render();

?>
