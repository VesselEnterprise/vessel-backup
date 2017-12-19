<?php

require_once 'core/common.inc.php';

if ( isset($_GET['action']) && $_GET['action'] == 'login' ) {
	
	$s = BackupSession::getSession();
	if ( $s->isLoggedIn() ) {
		die("User is already logged in");
	}
	
	echo "<div id=\"user_login\">";
	echo "<form action=\"" . $_SERVER["PHP_SELF"] . "\" method=\"POST\">";
	echo "<div><label for=\"user_name\">Username</label>";
	echo "<input type=\"text\" size=\"35\" name=\"user_name\" id=\"user_name\"></div>";
	echo "<div><label for=\"password\">Password</label>";
	echo "<input type=\"text\" size=\"35\" name=\"password\" id=\"password\"></div>";
	echo "<div><input type=\"submit\" value=\"Login\"></div>";
	echo "<input type=\"hidden\" name=\"action\" value=\"login\">";
	echo "</form>";
	echo "</div>";
	
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
	
	//Get Session Obj
	$s = BackupSession::getSession();
	if ( $s->login($user_name,$password) ) {
		echo "Login was successful!";
	}
	else {
		die("Error: Failed to authenticate. Please check your username or password.");
	}
	
}

if ( isset($_GET['action']) && $_GET['action'] == 'logout' ) {
	
	$s = BackupSession::getSession();
	$s->logout();
	
	echo "You have been successfully logged out<br/>";
	
}

if ( isset($_GET['action']) && $_GET['action'] == 'register' ) {
	
	echo "<div style=\"display: table\">";
	echo "<form action=\"" . $_SERVER["PHP_SELF"] . "\" method=\"POST\">";
	echo "<div><label for=\"user_name\">Username</label>";
	echo "<input type=\"text\" size=\"35\" name=\"user_name\" id=\"user_name\"></div>";
	echo "<div><label for=\"password\">Password</label>";
	echo "<input type=\"text\" size=\"35\" name=\"password\" id=\"password\"></div>";
	echo "<div><label for=\"email\">Email</label>";
	echo "<input type=\"text\" size=\"35\" name=\"email\" id=\"email\"></div>";
	echo "<div><label for=\"first_name\">First Name</label>";
	echo "<input type=\"text\" size=\"35\" name=\"first_name\" id=\"first_name\"></div>";
	echo "<div><label for=\"last_name\">Last Name</label>";
	echo "<input type=\"text\" size=\"35\" name=\"last_name\" id=\"last_name\"></div>";
	echo "<div><label for=\"title\">Job Title</label>";
	echo "<input type=\"text\" size=\"35\" name=\"title\" id=\"title\"></div>";
	echo "<div><label for=\"office\">Office</label>";
	echo "<input type=\"text\" size=\"35\" name=\"office\" id=\"office\"></div>";
	echo "<div><label for=\"address\">Address</label>";
	echo "<textarea size=\"35\" name=\"address\" id=\"address\"></textarea></div>";
	echo "<div><label for=\"city\">City</label>";
	echo "<input type=\"text\" size=\"35\" name=\"city\" id=\"city\"></div>";
	echo "<div><label for=\"state\">State</label>";
	echo "<input type=\"text\" size=\"35\" name=\"state\" id=\"state\"></div>";
	echo "<div><label for=\"zip\">Zip</label>";
	echo "<input type=\"text\" size=\"35\" name=\"zip\" id=\"zip\"></div>";
	echo "<div><input type=\"submit\" value=\"Register\"></div>";
	echo "<input type=\"hidden\" name=\"action\" value=\"register\">";
	echo "</form>";
	echo "</div>";
	
}

if ( isset($_POST['action']) && $_POST['action'] == "register" ) {
	
	if ( !isset($_POST['user_name']) || $_POST['user_name'] == "" ) {
		die("Error: No username was provided");
	}
	else if ( !isset($_POST['password']) || $_POST['password'] == "" ) {
		die("Error: No password was provided");
	}
	
	echo "Started user registration";
	
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
			BackupLog::getLog()->addMessage("User " . $username . " has been registered");
		}
		
		$stmt->close();
		
	}
	
}

?>