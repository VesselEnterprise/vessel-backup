<?php

	ini_set('display_errors', 1);
	error_reporting(E_ALL);

	$_PUT = array();

	function parseMultipartPut($data) {
		
		global $_PUT;
		
		//Split the data into two parts
		$parts = explode("\r\n\r\n", $data, 2);
		
		//Make sure we have two parts
		if ( count($parts) <= 1 )
			return;
		
		//Find the boundary
		$pattern = '/(Content-Type: multipart\/form-data; boundary=)(.+)/'; //\/form-data; boundary=)(.+)$
		preg_match($pattern, $parts[0], $matches );
		
		//$matches[2] should contain the boundary
		if ( !isset($matches[2]) )
			return;
		
		$boundary = "--" . trim($matches[2]);
		
		$multiparts = array_filter(explode( $boundary, trim($parts[1]) ));
		
		//Iterate through parts and create global keys for disposition names
		foreach( $multiparts as $key => $value ) {
			
			$arr = explode( "\r\n\r\n", $value );
			
			if ( count($arr) <= 1 )
				continue;
			
			//Parse the PUT variable name
			$pattern = '/(name=")(.+)(")/';
			preg_match($pattern, $value, $matches );
			
			if ( !isset($matches[2]) )
				continue;
			
			$_PUT[$matches[2]] = trim($arr[1]);
			
		}

	}

	$data = "POST /backup/api/v1/file HTTP/1.1
	Host: 10.1.10.208
	Authorization: 01580d2fa9741d42f6819127d74449fb
	Content-Disposition: multipart/form-data
	Cache-Control: no-cache
	Postman-Token: e824f48f-a996-1868-c385-673d6b60cab0
	Content-Type: multipart/form-data; boundary=----WebKitFormBoundary7MA4YWxkTrZu0gW
	\r\n\r\n
	------WebKitFormBoundary7MA4YWxkTrZu0gW
	Content-Disposition: form-data; name=\"metadata\"
	\r\n
	{
		\"compressed\": false,
		\"file_name\": \"exchange_mailbox2.png\",
		\"file_path\": \"/home/kyle/Pictures\",
		\"file_size\": 29543,
		\"file_type\": \".png\",
		\"hash\": \"61768db8d1a38f1c16d3e6eea812ef423c739068\",
		\"last_modified\": 1515437634
	}
	------WebKitFormBoundary7MA4YWxkTrZu0gW
	Content-Disposition: form-data; name=\"fileData\"
	\r\n
	ABC123
	------WebKitFormBoundary7MA4YWxkTrZu0gW--";

	parseMultipartPut($data);

	var_dump($_PUT);


?>