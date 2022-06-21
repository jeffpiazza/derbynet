<?php

require_once('inc/data.inc');

// $results, from parse_heat_results, is an array of {time, place}
function record_timer_finish($results) {
  global $db;

  $current = get_running_round();
  $roundid = @$current['roundid'];
  $heat = @$current['heat'];

  $stmt = $db->prepare('INSERT INTO TimerResults('
                       .' expected_roundid, expected_heat, lane,'
                       .' finishtime, finishplace)'
                       .' VALUES(:roundid, :heat, :lane, :finishtime, :finishplace)');
  $lane = 1;
  foreach($results as $result) {
    echo "<lane>$lane</lane>\n";
    try {
      if (!$stmt->execute(array(':roundid' => $roundid,
                                ':heat' => $heat,
                                ':lane' => $lane,
                                ':finishtime' => @$result['time'],
                                ':finishplace' => @$result['place']
                            ))) {
        // echo "<exec-fail>lane $lane</exec-fail>\n";
      }
    } catch (PDOException $p) {
      // xbecho "<fail>$p</fail>\n";
    }
    ++$lane;
  }
}

?>