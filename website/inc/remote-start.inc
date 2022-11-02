<?php

require_once('inc/data.inc');

function set_has_remote_start($v) {
    if ($v) {
        write_raceinfo('remote-start', time());
    } else {
        delete_from_raceinfo('remote-start');
    }
}

function has_remote_start() {
  return (read_raceinfo('remote-start', 0) + 60 >= time()) ? 1 : 0;
}

function queue_remote_start() {
  write_raceinfo('remote-start-go', 1);
}

function remote_start_pending() {
  $val = read_raceinfo('remote-start-go', 0);
  write_raceinfo('remote-start-go', 0);
  return $val;
}

?>
