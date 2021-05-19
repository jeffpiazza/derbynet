<?php

require_once('inc/remote-start.inc');

start_response();

if (have_permission(CONTROL_RACE_PERMISSION)) {
  if (has_remote_start()) {
    queue_remote_start();
    echo "  <success/>\n";
  } else {
    echo "  <failure code='noremotestart'>Remote start not available.</failure>\n";
  }
} else {
  not_authorized_body();
}

end_response();
?>