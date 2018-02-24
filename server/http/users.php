<?php

require_once 'core/common.inc.php';

$renderer = new PageRenderer();
$renderer->addTemplate('header');

if ( !$common['session']->isLoggedIn() ) {
  $renderer->addTemplate('login_form', array('refererUrl' => $common['session']->getRefererURL()) );
}


//Display a list of Users
if ( !isset($_GET['action']) && $common['session']->isLoggedIn() ) {

	//Get a count of total users for pagination
	$result = mysqli_query($common['dbconn'], "SELECT COUNT(*) FROM backup_user");
	$total_users = $result->fetch_row()[0];

	//Calculate results per page
	$results_per_page = 25;
	if ( isset($_GET['perPage']) )
		$results_per_page = (int)$_GET['perPage'];

	//Calculate total pages
	$totalPages = ceil($total_users / $results_per_page);

	//Calculate current page
	$currentPage=1;

	if ( isset($_GET['curPage']) )
		$currentPage = (int)$_GET['curPage'];

	//Sorting
	$orderBy = "user_name";
	$sortDirection = "ASC";

	if ( isset($_GET['orderBy']) )
		$orderBy = $_GET['orderBy'];

	if ( isset($_GET['sort']) )
		$sortDirection = $_GET['sort'];

	$resultStart = ($currentPage-1)*$results_per_page;
	$resultEnd = $currentPage*$results_per_page;

	$sqlOrderBy = $orderBy . " " . $sortDirection;

	//Query results
	$users=array();
	$query = "SELECT * FROM backup_user ORDER BY ? LIMIT ?,?";
	if ( $stmt = mysqli_prepare($common['dbconn'], $query) ) {
		$stmt->bind_param("sii", $sqlOrderBy, $resultStart, $resultEnd );
		$stmt->execute();

		$result = $stmt->get_result();
		$users = $result->fetch_all(MYSQLI_ASSOC);

		$stmt->close();
	}

	//Add user list template
	$vars = array("users" => $users, "totalPages" => $totalPages);
	$renderer->addTemplate('user_list', $vars);

}


$renderer->addTemplate('footer');
$renderer->render();

?>
