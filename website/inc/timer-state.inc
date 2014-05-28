<?php

require_once('inc/data.inc');

define('TIMER_NOT_CONNECTED', 1);
define('TIMER_CONNECTED', 2);
define('TIMER_PREPARED', 3);
define('TIMER_RUNNING', 4);

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
?>