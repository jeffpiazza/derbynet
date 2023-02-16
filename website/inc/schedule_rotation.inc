<?php

// For a two-lane track, a rotation schedule has no drawbacks.  As the number of
// lanes increase, there are more opportunities to face different competitors
// head-to-head using the normal scheduler; those opportunities are lost in a
// rotation schedule.
//
// The caller always ensures that $ncars >= $nlanes, by inserting byes as racers
// if needed.
function make_rotation_schedule($nlanes, $ncars, $n_times_per_lane, $trace) {
  $heats = array();
  // Running more than one heat per lane just repeats the same schedule.
  for ($repeat = 0; $repeat < $n_times_per_lane; ++$repeat) {
    $nheats = $ncars;  // max($ncars, $nlanes), but caller ensures $ncars >= $nlanes
    for ($pole = 0; $pole < $nheats; ++$pole) {
      $heat = array();
      // if $ncars < $nlanes, we need byes in the heat, not repeating the same car
      for ($lane = 0; $lane < $nlanes; ++$lane) {
        // The more "natural" expression, $pole + $lane mod $nheats, has the
        // cars shift left from heat to heat.  Using $nheats-$pole instead
        // shifts them right from heat to heat.
        $heat[] = ($nheats - $pole + $lane) % $nheats;
      }
      $heats[] = $heat;
    }
  }
  json_out('nheats', $nheats);
  json_out('ncars', $ncars);
  json_out('heats', $heats);
  return $heats;
}


?>
