<?php @session_start();

require_once('inc/authorize.inc');
session_write_close();
require_once('inc/export-all.inc');

require_permission(VIEW_RACE_RESULTS_PERMISSION);

header('Content-Type: application/json');

echo json_encode(export_all(), JSON_PRETTY_PRINT);
echo "\n";
?>
