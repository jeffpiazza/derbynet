<?php

// $_POST['recipient']
// $_POST['message']

require_once('inc/messages.inc');

send_message($_POST['recipient'], $_POST['message']);

purge_stale_messages();

json_success();

?>