<?php

function json_schedule($roundid) {
  global $db;
  $nlanes = get_lane_count();
  $race_chart = array();

  $stmt = $db->prepare('SELECT heat, lane, racerid'
                       .' FROM RaceChart'
                       .' WHERE roundid = :roundid'
                       .' ORDER BY heat, lane');
  $stmt->execute(array(':roundid' => $roundid));
  foreach ($stmt as $row) {
    while ($row['heat'] > count($race_chart)) {
      $race_chart[] = array_fill(0, $nlanes, null);
    }
    $race_chart[$row['heat'] - 1][$row['lane'] - 1] = intval($row['racerid']);
  }

  return $race_chart;
}
?>
