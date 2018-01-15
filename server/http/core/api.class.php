<?php

require_once 'api_session.class.php';
require_once 'file.class.php';
require_once 'user.class.php';
require_once 'upload.class.php';
require_once 'upload_part.class.php';

abstract class API
{
    /**
     * Property: method
     * The HTTP method this request was made in, either GET, POST, PUT or DELETE
     */
    protected $method = '';
    /**
     * Property: endpoint
     * The Model requested in the URI. eg: /files
     */
    protected $endpoint = '';
    /**
     * Property: verb
     * An optional additional descriptor about the endpoint, used for things that can
     * not be handled by the basic methods. eg: /files/process
     */
    protected $verb = '';
    /**
     * Property: args
     * Any additional URI components after the endpoint and verb have been removed, in our
     * case, an integer ID for the resource. eg: /<endpoint>/<verb>/<arg0>/<arg1>
     * or /<endpoint>/<arg0>
     */
    protected $args = Array();
    /**
     * Property: rawData
     * Stores raw data from a POST or PUT
     */
    protected $rawData = Null;
	 
	private $_db;
	private $_log;	
	private $_session; //API session object

    /**
     * Constructor: __construct
     * Allow for CORS, assemble and pre-process the data
     */
    public function __construct($request) {
		
		//Set output headers
        header("Access-Control-Allow-Orgin: *");
        header("Access-Control-Allow-Methods: *");
        header("Content-Type: application/json");
		
		//Detect endpoint and arguments
        $this->args = explode('/', rtrim($request, '/'));
        $this->endpoint = array_shift($this->args);
        if (array_key_exists(0, $this->args) && !is_numeric($this->args[0])) {
            $this->verb = array_shift($this->args);
        }
		
		//Set HTTP Method
		$this->method = $_SERVER['REQUEST_METHOD'];
        if ($this->method == 'POST' && array_key_exists('HTTP_X_HTTP_METHOD', $_SERVER)) {
            if ($_SERVER['HTTP_X_HTTP_METHOD'] == 'DELETE') {
                $this->method = 'DELETE';
            } else if ($_SERVER['HTTP_X_HTTP_METHOD'] == 'PUT') {
                $this->method = 'PUT';
            } else {
                throw new Exception("Unexpected Header");
            }
        }
		
		//Build objects
		$this->_db = BackupDatabase::getDatabase();
		//
		$this->_session = BackupAPISession::getSession();
		$this->_session->logSession($this->endpoint, $this->method);
		//
		$this->_log = BackupLog::getLog( $this->_session->getUserID() );

        switch($this->method) {
			case 'DELETE':
			case 'POST':
				$this->request = $this->_cleanInputs($_POST);
				$this->rawData = file_get_contents("php://input");
				break;
			case 'GET':
				$this->request = $this->_cleanInputs($_GET);
				break;
			case 'PUT':
				$this->request = $this->_cleanInputs($_GET);
				$this->rawData = file_get_contents("php://input");
				break;
			default:
				$this->_response('Invalid Method', 405);
				break;
        }
    }
	
	 public function processAPI() {
        if (method_exists($this, $this->endpoint)) {
            return $this->{$this->endpoint}($this->args);
        }
        return $this->_response("No Endpoint: $this->endpoint", 404);
    }

    private function _response($data, $status=200) {
        header("HTTP/1.1 " . $status . " " . $this->_requestStatus($status));
        return json_encode( array( 'response' => $data ), JSON_FORCE_OBJECT | JSON_PRETTY_PRINT );
    }
	
    private function _error($data, $status=200) {
		
		//Log Error
		$this->_log->addError($data,"API", 1 );		
		
        header("HTTP/1.1 " . $status . " " . $this->_requestStatus($status));
		
        return json_encode( array( 'error' => array('message' => $data, 'code' => $status) ), JSON_FORCE_OBJECT | JSON_PRETTY_PRINT );
		
    }

    private function _cleanInputs($data) {
        $clean_input = Array();
        if (is_array($data)) {
            foreach ($data as $k => $v) {
                $clean_input[$k] = $this->_cleanInputs($v);
            }
        } else {
            $clean_input = trim(strip_tags($data));
        }
        return $clean_input;
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
	
	private function test() {
		echo $this->_response("Hello World");
	}
	
	private function activate() {
		
		if ( $this->method != "POST" ) {
			echo $this->_error("This HTTP method is not supported", 405);
			return;
		}
		
		//Verify that the activation code is correct
		$request = json_decode($this->rawData, true );
		
		if ( empty($request['activation_code']) ) {
			echo $this->_error("Activation code is missing", 400);
			return;
		}
		
		if ( empty($request['user_name']) ) {
			echo $this->_error("Username is missing", 400);
			return;
		}
		
		$userName = trim($request['user_name']);
		$activationCode = trim($request['activation_code']);
		
		$user = new BackupUser($userName);
		$isActivated = $user->getUserData("access_token") ? true : false;
		
		if ( $accessToken = $user->activateUser($activationCode) ) {
			$isActivated=true;	
		}
		
		$msg = '';
		if ( !$accessToken && !$isActivated ) {
			$msg = "Error: User could not be activated";
		}
		else if(!$accessToken && $isActivated) {
			$msg = "User is already activated";
		}
		else {
			$msg = "User has been successfully activated";
		}

		echo $this->_response( array( 
			"user_name" => $userName,
			"access_token" => $accessToken,
			"is_activated" => $isActivated,
			"activation_code" => $activationCode,
			"message" => $msg 
		) );

	}
	
	/**
	* Returns client settings to the client
	*/
	private function settings() {
		
		if ( $this->method != "GET" ) {
			echo $this->_error("This method is not supported", 405 );
			return;			
		}
		
		if ( !$this->_session->isAuthenticated() ) {
			echo $this->_error("You are not authorized to perform this action", 401 );
			return;
		}
		
		$settings = array();
		
		if ( $result = mysqli_query($this->_db->getConnection(), "SELECT a.setting_id,a.name,a.value,a.data_type,b.setting_id,b.value AS override_value FROM backup_client_setting AS a LEFT JOIN backup_user_setting AS b ON a.setting_id = b.setting_id AND b.user_id = " . $this->_session->getUserID() ) ) {
			
			while ( $row = mysqli_fetch_array($result) ) {
				
				$val = !$row['override_value'] ? $row['value'] : $row['override_value'];
				
				$settings[$row['name']] = array();
				$settings[$row['name']]['value'] = $row['data_type'] == "int" ? (int)$val : (string)$val;
				$settings[$row['name']]['type'] = $row['data_type'];
				$settings[$row['name']]['user_override'] = !$row['override_value'] ? false : true;
					
			}
			
			$result->close();
			
		}		
			
		echo $this->_response( array( "settings" => $settings ), 200 );
		
	}
	
	private function user() {
		
		
	}
	
	private function file() {
		
		if ( !$this->_session->isAuthenticated() ) {
			echo $this->_error("You are not authorized to perform this action", 401 );
			return;			
		}
		
		if ( $this->method == "GET" ) {
			
			if ( empty($this->args[0]) ) {
				echo $this->_error("File ID is missing or does not exist", 400 );
				return;		
			}
			
			$file = new BackupFile($this->args[0]);
			
			if ( !$file->exists() ) {
				echo $this->_error("File ID does not exist", 400 );
				return;
			}
			
			echo $this->_response(array( 'file' => $file->getFile() ));
			
			return;			
		
		}
		
		if ( $this->method == "POST" ) {
			
			if ( empty($_POST['metadata']) ) {
				echo $this->_error("JSON metadata is missing", 400 );
				return;		
			}
			
			if ( empty($_POST['fileData']) ) {
				echo $this->_error("File contents are missing", 400 );
				return;		
			}
			
			$metadata = json_decode($_POST['metadata']);
			
			if ( json_last_error() != JSON_ERROR_NONE ) {
				echo $this->_error("JSON payload is invalid", 400 );
				return;		
			}
			
			$upload = new BackupUpload($metadata, $_POST['fileData'] );
			if ( !$upload->isValid() ) {
				echo $this->_error( $upload->getError(), 400 );
				return;
			}
			
			if ( $upload->upload() ) {
				echo $this->_response( array( 'file' => array('upload_id' => $upload->getUploadID() ) ) );
				return;
			}
			else {
				echo $this->_error( "There was a problem receiving the file: " . $upload->getError() );
				return;
			}
			
		}

	}
	
	private function file_part() {
		
		if ( !$this->_session->isAuthenticated() ) {
			echo $this->_error("You are not authorized to perform this action", 401 );
			return;			
		}
		
		//Initiate a new multi part upload
		if ( $this->method == "POST" ) {
			
			$payload = json_decode( $this->rawData );
			
			if ( json_last_error() != JSON_ERROR_NONE ) {
				echo $this->_error("JSON payload is invalid", 400 );
				return;		
			}
			
			$uploadPart = new BackupUploadPart();
			if ( !$uploadPart->initialize($payload) ) {
				echo $this->_error( $uploadPart->getError(), 400 );
				return;
			}
			
			echo $this->_response( array( "upload_id" => $uploadPart->getUploadID(), "file_id" => $uploadPart->getFileID() ) );
			
			return;
			
		}
		
		if ( $this->method == "PUT" ) {
			
			if ( empty($_PUT[0]['metadata']) ) {
				echo $this->_error("JSON metadata is missing", 400 );
				return;		
			}
			
			if ( empty($_PUT['fileData']) ) {
				echo $this->_error("File contents are missing", 400 );
				return;		
			}
			
			$metadata = json_decode( $_PUT['metadata'] );
			
			if ( json_last_error() != JSON_ERROR_NONE ) {
				echo $this->_error("JSON payload is invalid", 400 );
				return;		
			}
			
			var_dump($fileData);
			flush();
			return;
			
			$uploadPart = new BackupUploadPart();
			if ( !$uploadPart->uploadPart($metadata,$_POST['fileData']) ) {
				echo $this->_error( $uploadPart->getError(), 400 );
				return;
			}
			
		}
		
	}
	
	private function heartbeat() {
		
		if ( !$this->_session->isAuthenticated() ) {
			echo $this->_error("You are not authorized to perform this action", 401 );
			return;			
		}
		
		if ( $this->method != "POST" ) {
			echo $this->_error("Invalid request method", 400 );
			return;		
		}
		
		$payload = json_decode( $this->rawData, true );
		
		if ( json_last_error() != JSON_ERROR_NONE ) {
			echo $this->_error("JSON payload is invalid", 400 );
			return;		
		}
		
		$requiredFields = array(
			'host_name',
			'os',
			'client_version',
			'domain'
		);
		
		foreach ( $requiredFields as $key => $val ) {
			
			if ( !isset($payload[$val]) ) {
				echo $this->_error($val . " is missing from JSON payload", 400 );
				return;	
			}
			
		}
		
		$ts = time();
		$ip = $_SERVER['REMOTE_ADDR'];
		
		//Create or Update Client Machine
		$machineID = -1;
		$query = "INSERT INTO backup_machine (name,os,dns_name,ip_address,domain,client_version,last_check_in) VALUES(?,?,?,?,?,?,FROM_UNIXTIME(?)) ON DUPLICATE KEY UPDATE machine_id=LAST_INSERT_ID(machine_id),ip_address=?,domain=?,client_version=?";
		if ( $stmt = mysqli_prepare($this->_db->getConnection(), $query) ) {
			
			$stmt->bind_param(
				'ssssssisss',
				$payload['host_name'],
				$payload['os'],
				$payload['host_name'],
				$ip,
				$payload['domain'],
				$payload['client_version'],
				$ts,
				$ip,
				$payload['domain'],
				$payload['client_version']
			);
			
			if ( $stmt->execute() ) {
				
				$machineID = mysqli_insert_id($this->_db->getConnection());
				
			}
			
			$stmt->close();
			
		}
		
		$userID = $this->_session->getUserID();
		
		//Add or update user => machine association
		$query = "INSERT INTO backup_user_machine (machine_id,user_id,last_check_in) VALUES (?,?,FROM_UNIXTIME(?)) ON DUPLICATE KEY UPDATE last_check_in=FROM_UNIXTIME(?)";
		if ( $stmt = mysqli_prepare($this->_db->getConnection(), $query) ) {
			
			$stmt->bind_param('iiii', $machineID, $userID, $ts, $ts);
			$stmt->execute();		
			$stmt->close();
		}
		
		echo $this->_response( array("machine_id" => $machineID, "last_check_in" => $ts ) );
		
	}
	
}

?>
