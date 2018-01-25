<?php

class PageRenderer
{
	private $_pageTitle;
	private $_pageContent;
	
	function __construct() {
		
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
	
	public function addTemplate($templateName, $varMap=NULL) {
		
		ob_start();
			include ('templates/' . $templateName . '.tmpl');
			$content = ob_get_contents();
		ob_end_clean();
		
		$this->_pageContent .= $content;
		
	}

}

?>