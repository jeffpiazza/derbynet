<?php

require_once('inc/timer-state.inc');
require_once('inc/remote-start.inc');
require_once('inc/timer-state-xml.inc');  // For expand_timer_state_status


function json_timer_state() {
  $timer_state = new TimerState();
  list($msg, $icon) = expand_timer_state_status($timer_state);
  $tstate = $timer_state->state();

  return array(
    'lanes' => get_lane_count() + 0,
    'last-contact' => $timer_state->last_contact() + 0,
    'state' => $tstate,
    'icon' => $icon,
    'remote-start' => has_remote_start() ? true : false,
    'message' => $msg);
}

function json_timer_details() {
  return array('type' => read_raceinfo('timer-type', ''),
               'human' => read_raceinfo('timer-human', ''),
               'ident' => read_raceinfo('timer-ident', ''),
               'options' => read_raceinfo('timer-options', ''));
}

?>