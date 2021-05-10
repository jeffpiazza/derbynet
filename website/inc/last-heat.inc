<?php

// itemvalue is 50 chars for a GPRM database
// 7 chars per lane * 6 lanes = 42 chars, plus roundid # heat # (8), might just barely fit.

// When a heat result is recorded: last_heat = <roundid> # <heat>
// When a heat gets re-run, the results from the heat being rerun are captured:
//    last_heat = <roundid> # <heat> # <lane1> # <lane2> # ...
// where <laneN> is EITHER a finish time or a finish place, depending.

function record_last_heat($roundid, $heat) {
  write_raceinfo('last_heat', implode('#', array($roundid, $heat)));
}

function clear_last_heat() {
  global $db;
  $db->exec("DELETE FROM RaceInfo WHERE itemkey = 'last_heat'");
}

function capture_last_heat_results($roundid, $heat) {
  global $db;
  $last_heat = array($roundid, $heat);

  $result_column = read_raceinfo_boolean('use-points') ? 'finishplace' : 'finishtime';
  $stmt = $db->prepare('SELECT lane, '.$result_column.' AS result FROM RaceChart'
                       .' WHERE roundid = :roundid'
                       .'  AND  heat = :heat'
                       .' ORDER BY lane');
  $stmt->execute(array(':roundid' => $roundid,
                       ':heat' => $heat));
  $lane = 0;  // Last lane value written
  foreach ($stmt as $row) {
    while ($lane + 1 < $row['lane']) {
      $last_heat[] = '';
      $lane = $lane + 1;
    }
    $last_heat[] = $row['result'];
    $lane = $lane + 1;
  }

  write_raceinfo('last_heat', implode('#', $last_heat));
}

function get_last_heat() {
  return explode('#', read_raceinfo('last_heat', ''));
}

function last_heat_xml_element() {
  $last_heat = explode('#', read_raceinfo('last_heat', ''));
  if (count($last_heat) < 2) {
    echo "<last_heat type='none'/>";
  } else if (count($last_heat) == 2) {
    // The user could choose to re-run the last heat
    echo "<last_heat type='available'/>";
  } else {
    // The user could choose to reinstate the last heat
    echo "<last_heat type='recoverable'/>";
  }
}

?>