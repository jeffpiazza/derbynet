<?php

function assign_master_heats($round) {
  global $db;

  // Eventually, $weights is { roundid => weight }, where weight is an
  // integer, the number of heats from this roundid to include each
  // time through the loop.
  //
  // To compute $weights, first collect { roundid => total number of heats },
  // then divide (rounding) by the smallest number of heats.
  $weights = array();

  // $all_heats is { roundid => array of heats }, obtained from this
  // first query.
  $all_heats = array();

  $stmt = $db->prepare('SELECT roundid, heat'
                       .' FROM RaceChart'
                       .' WHERE roundid IN'
                       .'  (SELECT roundid FROM Rounds'
                       .'   WHERE round = :round)'
                       .' GROUP BY roundid, heat'
                       .' ORDER BY roundid, heat');
  $stmt->execute(array(':round' => $round));
  foreach ($stmt as $row) {
    $roundid = $row[0];
    $heat = $row[1];
    if (!isset($all_heats[$roundid])) {
      $all_heats[$roundid] = array();
      $weights[$roundid] = 0;
    }
    $all_heats[$roundid][] = $heat;
    ++$weights[$roundid];
    // echo '<heat roundid="'.$roundid.'" heat="'.$heat.'"/>'."\n";
  }

  // Normalize weights
  $min_count = min($weights);
  foreach ($weights as $roundid => $w) {
    $weights[$roundid] = round($w / $min_count);
  }

  $masterheat = 1;
  $flag = true;
  while ($flag) {
    $flag = false;
    foreach ($all_heats as $roundid => &$heats) {
      $weight = $weights[$roundid];
      for ($c = 0; $c < min($weight, count($heats)); ++$c) {
        $heat = array_shift($heats);
        // echo '<update roundid="'.$roundid.'" heat="'.$heat.'" masterheat="'.$masterheat.'"/>'."\n";
        $stmt = $db->prepare('UPDATE RaceChart'
                             .' SET masterheat = :masterheat'
                             .' WHERE roundid = :roundid'
                             .' AND heat = :heat');
        $stmt->execute(array(':masterheat' => $masterheat,
                             ':roundid' => $roundid,
                             ':heat' => $heat));
        $flag = true;
        ++$masterheat;
      }
    }
  }
}
?>