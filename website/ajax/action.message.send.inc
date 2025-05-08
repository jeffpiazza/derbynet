<?php

// $_POST['message']

require_once('inc/messages.inc');

$msg = $_POST['message'];


if (isset($_POST['recipient'])) {
  // This form isn't currently used
  send_message($_POST['recipient'], $_POST['message']);
} else {
  $recipient = isset($msg['recipient']) ? $msg['recipient'] : "";
  send_message($recipient, json_encode($msg));
}

purge_stale_messages();

json_success();

?>
