<?php
require_once('inc/replay.inc');

start_response();

if (have_permission(CONTROL_RACE_PERMISSION)) {
  send_replay_REPLAY();
  echo "<success/>\n";
} else {
  not_authorized_body();
}

end_response();
?>