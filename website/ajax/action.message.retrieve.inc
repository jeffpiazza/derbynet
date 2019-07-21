<?php

// $_POST['recipient']

require_once('inc/messages.inc');

start_response();

purge_stale_messages();

foreach (retrieve_messages($_POST['recipient']) as $msg) {
  echo "<message>".htmlspecialchars($msg, ENT_QUOTES, 'UTF-8')."</message>\n";
}
echo "<success/>\n";

end_response();
