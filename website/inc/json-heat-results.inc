<?php

function json_heat_results($roundid, $heat) {
  global $db;

  $track_length = read_raceinfo('track-length', 40);  // in feet
  $time_format = get_finishtime_formatting_string();

// $track_length / $finishtime gives feet per second.
//  * 3600 gives feet per hour
//  / 5280 gives miles per hour
//  * 25 gives scale miles per mile (not user-adjustable; makes a 7" derby car scale to 14.5 feet)
  $fps_to_scale_mph = 3600 * 25 / 5280;

  $stmt = $db->prepare('SELECT lane, finishtime, finishplace, completed'
                       .' FROM RaceChart'
                       .' WHERE roundid = :roundid'
                       .' AND heat = :heat'
                       //.' AND completed IS NOT NULL AND completed != \'\''
                       .' AND (finishtime IS NOT NULL OR finishplace IS NOT NULL)'
                       .' ORDER BY lane');
  $stmt->execute(array(':roundid' => $roundid,
                       ':heat' => $heat));
  $heat_results = array();
  foreach ($stmt as $row) {
    $finishtime = $row['finishtime'];
    $speed = (!is_numeric($finishtime) || $finishtime == 0) ? "--" : sprintf("%4.1f", $track_length / $finishtime * $fps_to_scale_mph);
    $heat_results[] = array(
      'lane' => $row['lane'],
      'time' => sprintf($time_format, $finishtime),
      'place' => $row['finishplace'],
      'speed' => $speed);
  }
  return $heat_results;
}

?>
