<?php

	var_dump($_POST);

	$output = file_get_contents('php://input');

	echo $output;

?>