<?php

require_once('inc/timer-state.inc');

start_response();

if (have_permission(TIMER_MESSAGE_PERMISSION)) {
  $timer_state = new TimerState();
  $timer_state->set_assume_connected_for_test();
  $timer_state->write(false);
  echo "\n<success/>\n";
} else {
	not_authorized_body();
}

end_response();

?>