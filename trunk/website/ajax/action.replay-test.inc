<?php
require_once('inc/replay.inc');

start_response();

send_replay_TEST();
echo "<success/>\n";

end_response();
?>