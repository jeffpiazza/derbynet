<?php
require_once('inc/timer-state.inc');
require_once('inc/remote-start.inc');

// Returns two values, message and icon
function expand_timer_state_status($timer_state) {
  $tstate = $timer_state->state();
  if ($tstate == TIMER_NOT_CONNECTED) {
    return array('NOT CONNECTED', 'img/status/not_connected.png');
  } else if ($tstate == TIMER_SEARCHING) {
    return array('Scanning for timer', 'img/status/unknown.png');
  } else if ($timer_state->troubled()) {
    return array($timer_state->malfunction(), 'img/status/trouble.png');
  } else if ($tstate == TIMER_CONNECTED) {
    return array('CONNECTED ('.($timer_state->seconds_since_last_contact()).' sec.)',
                 'img/status/ok.png');
  } else if ($tstate == TIMER_STAGING) {
    return array('Staging', 'img/status/ok.png');
  } else if ($tstate == TIMER_RUNNING) {
    return array('Race running', 'img/status/ok.png');
  } else if ($tstate == TIMER_UNHEALTHY) {
    return array('UNHEALTHY', 'img/status/trouble.png');
  } else if ($tstate == TIMER_UNCONFIRMED) {
    return array('UNCONFIRMED', 'img/status/unknown.png');
  } else {
    return array('Unknown ('.$tstate.')', 'img/status/trouble.png');
  }
}

function emit_timer_state() {
  $timer_state = new TimerState();
  list($msg, $icon) = expand_timer_state_status($timer_state);
  $tstate = $timer_state->state();

  echo '<timer-state lanes="'.get_lane_count().'"'
      .' last_contact="'.$timer_state->last_contact().'"'
      .' state="'.$tstate.'"'
      .' icon="'.$icon.'"'
      .' remote_start="'.has_remote_start().'"'
      .'>';
  echo $msg;
  echo '</timer-state>'."\n";
}

function emit_timer_details() {
  echo "<timer-details";
  echo " type=\"".htmlspecialchars(read_raceinfo('timer-type', ''), ENT_QUOTES, 'UTF-8')."\"";
  echo " human=\"".htmlspecialchars(read_raceinfo('timer-human', ''), ENT_QUOTES, 'UTF-8')."\"";
  echo " ident=\"".htmlspecialchars(read_raceinfo('timer-ident', ''), ENT_QUOTES, 'UTF-8')."\"";
  // echo " options=\"".htmlspecialchars(read_raceinfo('timer-options', ''), ENT_QUOTES, 'UTF-8')."\"";
  echo "/>\n";
}
?>