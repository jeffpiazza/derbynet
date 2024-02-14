<?php

// $_POST['message']

require_once('inc/messages.inc');

$msg = $_POST['message'];
$recipient = isset($msg['recipient']) ? $msg['recipient'] : "";

send_message($recipient, json_encode($msg));

purge_stale_messages();

json_success();

?>
