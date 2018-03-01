<?php

require_once 'core/common.inc.php';

$renderer = new PageRenderer();
$renderer->addTemplate('header');

if ( !$common['session']->isLoggedIn() ) {
  $renderer->addTemplate('login_form', array('refererUrl' => $common['session']->getRefererURL()) );
}

//Display a list of Settings
$settings = array();
$categories = array();
if ( !isset($_GET['action']) && $common['session']->isLoggedIn() ) {

  //Get setting categories
  $result = mysqli_query($common['dbconn'], "SELECT DISTINCT(type) FROM backup_setting WHERE active=1");
  $categories = $result->fetch_all(MYSQLI_ASSOC);

  $result->close();

  //Get Individual Settings
  $selectedCategory = 'Database'; //Default
  if ( isset($_GET['category']) ) {
    $selectedCategory = $_GET['category'];
  }

  $query = "SELECT * FROM backup_setting WHERE active=1 AND type=?";
  if ( $stmt = mysqli_prepare($common['dbconn'], $query) ) {

      $stmt->bind_param('s', $selectedCategory);

      if ( $stmt->execute() ) {

        $result = $stmt->get_result();
        $settings = $result->fetch_all(MYSQLI_ASSOC);

      }
      else {
        echo mysqli_error($common['dbconn']);
      }

      $stmt->close();

  }
  else {
    echo mysqli_error($common['dbconn']);
  }

  //Add settings to renderer
  $renderer->addTemplate(
    'setting_list',
    array(
      "categories" => $categories,
      "settings" => $settings,
      "selectedCategory" => $selectedCategory
    )
  );

}

$renderer->addTemplate('footer');
$renderer->render();

?>
