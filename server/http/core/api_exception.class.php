<?php

require_once 'api_session.class.php';
require_once 'log.class.php';

class APIException extends Exception {

  public function __construct($message, $statusCode=400) {

      $session = BackupAPISession::getSession();
      $log = BackupLog::getLog($session->getUserId());

  		//Save error to db
  		$log->addError( $message, "API", 1 );

      //Send the proper headers to the client
      header("HTTP/1.1 " . $statusCode . " " . $this->_requestStatus($statusCode));

      //Output JSON error response to client
      echo (json_encode( array( 'error' => array('message' => $message, 'code' => $statusCode) ), JSON_FORCE_OBJECT | JSON_PRETTY_PRINT ));

  }

  private function _requestStatus($code) {

      $status = array(
          200 => 'OK',
    			400 => 'Bad Request',
    			401 => 'Not Authorized',
    			403 => 'Forbidden',
          404 => 'Not Found',
          405 => 'Method Not Allowed',
          500 => 'Internal Server Error',
      );

      return ($status[$code]) ? $status[$code] : $status[500];

  }

}

?>
