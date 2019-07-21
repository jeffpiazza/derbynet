<?php

// $_POST['recipient']
// $_POST['message']

start_response();

require_once('inc/messages.inc');

send_message($_POST['recipient'], $_POST['message']);

echo "\n<success/>\n";

purge_stale_messages();

end_response();

?>