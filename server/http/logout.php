<?php

require_once 'core/common.inc.php';

if ( $common['session']->logout() ) {

  echo "You have been successfully logged out<br/>";

}

?>
