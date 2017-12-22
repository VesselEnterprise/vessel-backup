<?php

require_once 'common.inc.php';
require_once 'user.class.php';

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

    /**
     * Constructor: __construct
     * Allow for CORS, assemble and pre-process the data
     */
    public function __construct($request) {
		
		$this->_db = BackupDatabase::getDatabase();
		
        header("Access-Control-Allow-Orgin: *");
        header("Access-Control-Allow-Methods: *");
        header("Content-Type: application/json");

        $this->args = explode('/', rtrim($request, '/'));
        $this->endpoint = array_shift($this->args);
        if (array_key_exists(0, $this->args) && !is_numeric($this->args[0])) {
            $this->verb = array_shift($this->args);
        }

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
            return $this->_response($this->{$this->endpoint}($this->args));
        }
        return $this->_response("No Endpoint: $this->endpoint", 404);
    }

    private function _response($data, $status = 200) {
        header("HTTP/1.1 " . $status . " " . $this->_requestStatus($status));
		header("Content-Type: application/json");
        return json_encode( array( 'response' => $data ), JSON_PRETTY_PRINT );
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
			echo $this->_response("Error: This HTTP method is not supported");
			return;
		}
		
		//Verify that the activation code is correct
		$request = json_decode($this->rawData, true );
		
		if ( empty($request['activation_code']) ) {
			echo $this->_response("Error: Activation code is missing");
			return;
		}
		
		if ( empty($request['user_name']) ) {
			echo $this->_response("Error: Username is missing");
			return;
		}
		
		$userName = trim($request['user_name']);
		$activationCode = trim($request['activation_code']);
		
		$user = new BackupUser($userName);
		$accessToken = $user->activateUser($activationCode);
		
		$msg = '';
		if ( !$accessToken ) {
			$msg = "Error: User could not be activated";
		}
		else {
			$msg = "User has been successfully activated";
		}
		
		echo $this->_response( array( 
			"user_name" => $userName,
			"access_token" => $accessToken,
			"activation_code" => $activationCode,
			"message" => $msg 
		) );

	}
	
	private function authenticate() {
		
		//Verify that the provided access token and user name are correct
		$request = json_decode($this->rawData, true );
		
		if ( empty($request['user_name']) ) {
			echo $this->_request("Error: Username is missing");
			return;
		}
		
		if ( empty($request['access_token']) ) {
			echo $this->_request("Error: Access token is missing");
			return;
		}
		
		$query = "SELECT user_id,access_token FROM backup_user WHERE user_name=? AND access_token=?";
		if ( $stmt = mysqli_prepare($this->_db->getConnection(), $query) ) {
			
			$username = $request['user_name'];
			$access_token = $request['access_token'];
			
			$stmt->bind_param('ss', $username, $access_token );
			
			if ( $stmt->execute() ) {
				$result = $stmt->get_result();
				if ( $result->num_rows > 0 ) {
					
					//Check if access token is empty, if so then generate a new one
					if ( $result[1] == "" ) {
						$access_token = bin2hex(random_bytes(32));
					}
					
				}
				
			}
			
			$stmt->close();
			
		}
		
	}
	
	private function user() {
		
		
	}
	
	private function file() {
		
		echo $this->_response( array( 'file_id' => 100 ) );
		
		
	}
}

?>