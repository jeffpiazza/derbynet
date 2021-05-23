<?php

// $_POST['recipient']

require_once('inc/messages.inc');

purge_stale_messages();

$messages = array();
foreach (retrieve_messages($_POST['recipient']) as $msg) {
  $messages[] = json_decode($msg);
}

json_out('messages', $messages);

json_success();
