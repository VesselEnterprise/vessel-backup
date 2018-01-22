<?php

require_once 'common.inc.php';

class BackupLDAP
{
	
	private $_lc; //LDAP connection handle
	private $_host;
	private $_port;
	private $_userdn;
	private $_pwd;
	
	private $_db;
	private $_dbconn;
	private $_log;
	private $_session;
	
	public function __construct() {
		
		//Connect to MySQL database
		$this->_db = BackupDatabase::getDatabase();
		$this->_dbconn = $this->_db->getConnection();
		$this->_session = BackupSession::getSession();
		
		//Create new log object
		$this->_log = BackupLog::getLog( $this->_session->getUserId() );
		
		$this->_host = $this->_db->getSetting('ldap_server');
		$this->_port = $this->_db->getSetting('ldap_port');
		$this->_userdn = $this->_db->getSetting('ldap_user');
		$this->_pwd = $this->_db->getSetting('ldap_pwd');
		
		if ( $this->_connect() ) {
			if ( !$this->_bind() ) {
				$this->_log->addMessage( "Unable to bind to LDAP server", "LDAP" );
			}
		}
		else {
			$this->_log->addMessage( "Unable to connect to LDAP server", "LDAP" );
		}
		
	}
	
	function __destruct() {
		
		ldap_unbind($this->_lc);
		
		echo "Unbind successfully";
		
	}
	
	private function _connect() {

		$this->_lc = ldap_connect($this->_host, $this->_port);
		
		if ( !$this->_lc ) {
			$this->_log->addMessage( "Unable to connect to LDAP server", "LDAP" );
			return false;
		}
		
		ldap_set_option($this->_lc, LDAP_OPT_PROTOCOL_VERSION, 3); 
		ldap_set_option($this->_lc, LDAP_OPT_REFERRALS, 0); 
		
		$this->_log->addMessage( "Connected to LDAP server successfully", "LDAP" );
		
		return true;

	}
	
	private function _bind() {

		if ( !ldap_bind($this->_lc, $this->_userdn, $this->_pwd) )
			return false;
		
		return true;
	}
	
	public function importUsers() {
		
		$timeStart = microtime(TRUE);
		
		$this->_log->addMessage( "Started LDAP User Import", 'LDAP');
		
		if ( !$this->_lc ) {
			//Handle error
			return;
		}
		
		$dn = $this->_db->getSetting("ldap_user_tree");
		$filter = "(&(objectClass=person)(sn=*)(!(userAccountControl:1.2.840.113556.1.4.803:=2)))";
		$attributes = array(
			"sAMAccountName", 
			"sn", 
			"givenname", 
			"mail", 
			"title", 
			"streetAddress", 
			"l", 
			"st", 
			"postalCode", 
			"physicalDeliveryOfficeName", 
			"mobile"
		);
		
		//Enable support for retrieving more than 5000 (or the default LDAP max per page)
		$pageSize = 1000;
		$ldapCookie = '';
		
		$userSrc = "LDAP";
		$ldapActivateCode = $this->_db->getSetting("ldap_activate_code");
		
		do {
		
			ldap_control_paged_result($this->_lc, $pageSize, true, $ldapCookie);

			$users = ldap_search($this->_lc, $dn, $filter, $attributes );
			
			if ( !$users ) {
				$this->_log->addMessage( "LDAP user search failed (" . ldap_error($this->_lc) . ")", "LDAP" );
				return;
			}

			$entries = ldap_get_entries($this->_lc, $users);
			$userTotal = $entries["count"];
			
			//Log LDAP Import User Count
			$this->_log->addMessage( $userTotal ." entries returned", 'LDAP');

			$query = 
				"INSERT INTO backup_user (user_name,first_name,last_name,email,address,city,state,zip,title,office,mobile,source) " .
				"VALUES (?,?,?,?,?,?,?,?,?,?,?,?) ON DUPLICATE KEY UPDATE user_name=?,email=?,address=?,city=?,state=?,zip=?,title=?,office=?,mobile=?,user_id=LAST_INSERT_ID(user_id)";
				
			if ( $stmt = mysqli_prepare($this->_dbconn, $query) ) {

				//Add users to database
				for ( $i=0; $i < $userTotal; $i++ ) {
					
					//var_dump($entries[$i]);
						
					$stmt->bind_param(
						'sssssssssssssssssssss', //Last entry should always be user_name (pkey)
						$entries[$i]['samaccountname'][0],
						$entries[$i]['givenname'][0],
						$entries[$i]['sn'][0],
						$entries[$i]['mail'][0],
						$entries[$i]['streetaddress'][0],
						$entries[$i]['l'][0],
						$entries[$i]['st'][0],
						$entries[$i]['postalcode'][0],
						$entries[$i]['title'][0],
						$entries[$i]['physicaldeliveryofficename'][0],
						$entries[$i]['mobile'][0],
						$userSrc,
						$entries[$i]['samaccountname'][0],
						$entries[$i]['mail'][0],
						$entries[$i]['streetaddress'][0],
						$entries[$i]['l'][0],
						$entries[$i]['st'][0],
						$entries[$i]['postalcode'][0],
						$entries[$i]['title'][0],
						$entries[$i]['physicaldeliveryofficename'][0],
						$entries[$i]['mobile'][0]
					);
					
					if ( $stmt->execute() ) {
						echo "User " . $entries[$i]['samaccountname'][0] . " has been added to database<br/>";
						flush();
						
						$userId = mysqli_insert_id($this->_dbconn);
						
						//Create new user activation record?
						
						//Get user object
						$user = BackupUser::getUser($userId);
						
						//If user has no access token, create a new activation record
						if ( !$user->getValue('access_token') ) {
							$user->createUserActivation( $ldapActivateCode );
						}
						
					}
					else {
						$this->_log->addMessage( "Failed to add user to database: " . mysqli_error($this->_dbconn), "LDAP" );
					}
					
				}
			
				$stmt->close();
				
			}
			else {
				$this->_log->addMessage( "Failed to prepare SQL statement: " . mysqli_error($this->_dbconn), "LDAP" );
			}
			
			ldap_control_paged_result_response($this->_lc, $users, $ldapCookie);
       
		} while( $ldapCookie !== null && $ldapCookie != '');
		
		$timeEnd = microtime(TRUE);
		
		$totalTime = $timeEnd - $timeStart;
		
		$this->_log->addMessage( "LDAP user import finished (" . $totalTime . "ms)", "LDAP" );
		
	}
	
public function importComputers() {
		
		$timeStart = microtime(TRUE);
		
		$this->_log->addMessage( "Started LDAP Computer Import", 'LDAP');
		
		if ( !$this->_lc ) {
			//Handle error
			return;
		}
		
		$dn = $this->_db->getSetting("ldap_pc_tree");
		$filter = "(&(objectCategory=computer))";
		$attributes = array(
			"name", 
			"dNSHostName", 
			"operatingSystem"
		);
		
		//Enable support for retrieving more than 5000 (or the default LDAP max per page)
		$pageSize = 1000;
		$ldapCookie = '';
		
		do {
		
			ldap_control_paged_result($this->_lc, $pageSize, true, $ldapCookie);

			$computers = ldap_search($this->_lc, $dn, $filter, $attributes );
			
			if ( !$computers ) {
				$this->_log->addMessage( "LDAP computer search failed (" . ldap_error($this->_lc) . ")", "LDAP" );
				return;
			}

			$entries = ldap_get_entries($this->_lc, $computers);
			$computerTotal = $entries["count"];
			
			//Log LDAP Import User Count
			$this->_log->addMessage( $computerTotal ." entries returned", 'LDAP');
			
			$computerSrc = "LDAP";
			
			$query = "INSERT INTO backup_machine (name,os,dns_name,source) VALUES(?,?,?,?) ON DUPLICATE KEY UPDATE name=?";
				
			if ( $stmt = mysqli_prepare($this->_dbconn, $query) ) {

				//Add computers to database
				for ( $i=0; $i < $computerTotal; $i++ ) {
					
					//var_dump($entries[$i]);
						
					$stmt->bind_param(
						'sssss', //Last entry should always be user_name (pkey)
						$entries[$i]['name'][0],
						$entries[$i]['operatingsystem'][0],
						$entries[$i]['dnshostname'][0],
						$computerSrc,
						$entries[$i]['name'][0]
					);
					
					if ( $stmt->execute() ) {
						echo "Computer " . $entries[$i]['name'][0] . " has been added to database<br/>";
						flush();
					}
					else {
						$this->_log->addMessage( "Failed to add computer to database: " . mysqli_error($this->_dbconn), "LDAP" );
					}
					
				}
			
				$stmt->close();
				
			}
			else {
				$this->_log->addMessage( "Failed to prepare SQL statement: " . mysqli_error($this->_dbconn), "LDAP" );
			}
			
			ldap_control_paged_result_response($this->_lc, $computers, $ldapCookie);
       
		} while( $ldapCookie !== null && $ldapCookie != '');
		
		$timeEnd = microtime(TRUE);
		
		$totalTime = $timeEnd - $timeStart;
		
		$this->_log->addMessage( "LDAP computer import finished (" . $totalTime . "ms)", "LDAP" );
		
	}
	
}

?>