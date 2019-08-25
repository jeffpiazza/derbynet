<?php

// Re-compute masterheat indices for all the heats in Round $round that haven't
// yet been run.
//
// There may already be completed heats in the round (especially if we're
// assigning master heats due to the creation of an aggregate round); we want
// to be sure not to change masterheat values for those RaceChart entries.
function assign_master_heats($round) {
  global $db;

  // Eventually, $weights is { roundid => weight }, where weight is an
  // integer, the number of heats from this roundid to include each
  // time through the loop.
  //
  // To compute $weights, first collect { roundid => total number of heats } in
  // $weights, then divide (rounding) by the smallest number of heats.
  $weights = array();

  // $all_heats is { roundid => array of heats }, obtained from this
  // first query.
  $all_heats = array();

  $stmt = $db->prepare('SELECT DISTINCT roundid, heat'
                       .' FROM RaceChart'
                       .' WHERE roundid IN'
                       .'  (SELECT roundid FROM Rounds'
                       .'   WHERE round = :round)'
                       .' AND finishtime IS NULL AND finishplace IS NULL'
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

  $masterheat = 1 +
      read_single_value('SELECT MAX(masterheat)'
                        .' FROM RaceChart'
                        .' WHERE roundid IN'
                        .'  (SELECT roundid FROM Rounds'
                        .'   WHERE round < :round)'
                        .' OR (roundid IN (SELECT roundid FROM Rounds WHERE round = :round2)'
                        .'     AND (finishtime IS NOT NULL OR finishplace IS NOT NULL))',
                        array(':round' => $round,
                              ':round2' => $round),
                        /* default */ 0);
  $flag = true;
  while ($flag) {
    $flag = false;
    foreach ($all_heats as $roundid => &$heats) {
      $weight = $weights[$roundid];
      $heats_left = count($heats);
      for ($c = 0; $c < min($weight, $heats_left); ++$c) {
        $heat = array_shift($heats);
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