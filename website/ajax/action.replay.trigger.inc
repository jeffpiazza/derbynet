<?php
require_once('inc/replay.inc');

if (have_permission(CONTROL_RACE_PERMISSION)) {
  send_replay_REPLAY();
  json_success();
} else {
  not_authorized_body();
}

?>