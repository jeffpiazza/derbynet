<?php
require_once('inc/replay.inc');

if (have_permission(CONTROL_RACE_PERMISSION)) {
  send_replay_TEST();
  json_success();
} else {
  json_not_authorized();
}
?>