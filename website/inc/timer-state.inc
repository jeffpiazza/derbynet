<?php

require_once('inc/data.inc');

define('TIMER_NOT_CONNECTED', 1);  // No recent contact from an authenticated timer
define('TIMER_CONNECTED', 2);      // Timer connected, but not doing anything right now
define('TIMER_PREPARED', 3);       // A HEAT-READY message sent to connected timer
define('TIMER_RUNNING', 4);        // A STARTED message received (but not yet a FINISHED message)

define('TIMER_MALFUNCTION_AUTO', 5); // A persistently-detectable malfunction;
                                     // will clear itself (good heartbeat) when
                                     // the problem is corrected.
define('TIMER_MALFUNCTION_MANUAL', 6);
                                     // An intermittently-detectable
                                     // malfunction; race coordinator must clear
                                     // the condition, because heartbeat doesn't
                                     // mean it's fixed.

function get_timer_state() {
  if (read_raceinfo('timer_last_contact', 0) + 60 /* seconds */ < time()) {
    return TIMER_NOT_CONNECTED;
  }
  return read_raceinfo('timer_state', TIMER_NOT_CONNECTED);
}

function set_timer_state($timer_state) {
  write_raceinfo('timer_state', $timer_state);
  write_raceinfo('timer_last_contact', time());
}

function renew_timer_contact() {
  write_raceinfo('timer_last_contact', time());
}
?>