<?php

ini_set('display_errors', 1);
error_reporting(E_ALL);

require_once 'core/page_renderer.class.php';

$renderer = new PageRenderer();

$renderer->addTemplate('header');
$renderer->addTemplate('footer');

$renderer->render();

?>