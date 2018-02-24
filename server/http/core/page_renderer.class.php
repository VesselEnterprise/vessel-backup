<?php

class PageRenderer
{
	private $_pageTitle;
	private $_pageContent = '';

	function __construct() {

	}

	function __destruct() {
		//ob_end_clean();
	}

	public function setPageTitle($title) {
		$this->_pageTitle = $title;
	}

	public function addContent($content) {
		$this->_pageContent .= $content;
	}

	public function render() {
		echo $this->_pageContent;
	}

	public function addTemplate($templateName, $vars=null) {

		ob_start();

		if ( isset($vars) )
			extract($vars, EXTR_SKIP);

		include ('templates/' . $templateName . '.tmpl');

		$this->_pageContent .= ob_get_clean();

	}

}

?>
