<?php @session_start();
require_once('inc/data.inc');
require_once('inc/authorize.inc');
require_once('inc/export-results.inc');

header('Content-Type: text/csv; charset=utf-8');
header('Content-Disposition: attachment; filename="derbynet-'.date('Y-m-d').'.csv"');

$name_style = isset($_GET['last_initial']) ? FIRST_NAME_LAST_INITIAL : FULL_NAME;

$output = fopen("php://output", "w");

export_results_csv($output, $name_style);

?>
