<?php
require_once('inc/json-awards.inc');

$adhoc = isset($_GET['adhoc']) ? $_GET['adhoc'] : 1;
json_out_awards($adhoc);
?>
