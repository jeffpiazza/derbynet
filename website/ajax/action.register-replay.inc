<?php
// $_POST['port']
//
// TODO: Permissions?  Presently accepting registration action without authentication.

require_once('inc/replay.inc');

start_response();

write_replay_host_and_port($_SERVER['REMOTE_ADDR'].':'.$_POST['port']);

echo "<success/>\n";

end_response();
?>