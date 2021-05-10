<?php

require_once('inc/last-heat.inc');


function json_last_heat() {
  $last_heat = explode('#', read_raceinfo('last_heat', ''));
  if (count($last_heat) < 2) {
    return 'none';
  } else if (count($last_heat) == 2) {
    return 'available';
  } else {
    // The user could choose to reinstate the last heat
    return 'recoverable';
  }
}

?>