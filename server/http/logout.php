<?php

require_once 'core/common.inc.php';

$renderer = new PageRenderer();
$renderer->addTemplate('header');

if ( $common['session']->logout() ) {

  $renderer->addTemplate('generic_message', array("message" => "You have been successfully logged out<br/>"));

}

$renderer->addTemplate('footer');
$renderer->render();

?>
