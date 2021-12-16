<?php

require_once('inc/remote-start.inc');

if (have_permission(CONTROL_RACE_PERMISSION)) {
  if (has_remote_start()) {
    queue_remote_start();
    json_success();
  } else {
    json_failure('no-remote-start', "Remote start not available.");
  }
} else {
  json_not_authorized();
}

?>