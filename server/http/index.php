<?php

ini_set('display_errors', 1);
error_reporting(E_ALL);

require_once 'core/common.inc.php';

$renderer = new PageRenderer();

$renderer->addTemplate('header');

if ( !$common['session']->isLoggedIn() ) {
  $renderer->addTemplate('login_form', array('refererUrl' => $common['session']->getRefererURL()) );
}

$renderer->addTemplate('footer');

$renderer->render();

?>
