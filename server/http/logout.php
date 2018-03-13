<?php

require_once 'core/common.inc.php';

$common['session']->logout();

$renderer = new PageRenderer();
$renderer->addTemplate('header');
$renderer->addTemplate('generic_message', array("message" => "You have been successfully logged out<br/>"));
$renderer->addTemplate('footer');
$renderer->render();

?>
