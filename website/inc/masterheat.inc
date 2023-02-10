<?php


// Recompute masterheat indices for all heats not yet run.  Order by round
// level, so any unraced round 1 heats are scheduled before any unraced round 2
// heats.
function assign_master_heats() {
  global $db;

  $max_round_level = read_single_value('SELECT MAX(round) FROM Rounds', array(), 1);

  $masterheat = 1 +
              read_single_value('SELECT MAX(masterheat)'
                                .' FROM RaceChart WHERE finishtime IS NOT NULL'
                                .' OR finishplace IS NOT NULL',
                                array(), /* default */ 0);

  $get_heat_stmt = $db->prepare('SELECT DISTINCT roundid, heat'
                                .' FROM RaceChart'
                                .' WHERE roundid IN'
                                .'  (SELECT roundid FROM Rounds'
                                .'   WHERE round = :round)'
                                .' AND finishtime IS NULL AND finishplace IS NULL'
                                .' ORDER BY roundid, heat');
  
  $upd_stmt = $db->prepare('UPDATE RaceChart'
                           .' SET masterheat = :masterheat'
                           .' WHERE roundid = :roundid'
                           .' AND heat = :heat');

  for ($round_level = 1; $round_level <= $max_round_level; ++$round_level) {
    // Eventually, $weights is { roundid => weight } for each roundid at the
    // current $round_level, where weight is an integer, the number of heats
    // from this roundid to include each time through the loop.
    //
    // To compute $weights, first collect { roundid => total number of heats } in
    // $weights, then divide (rounding) by the smallest number of heats.
    $weights = array();

    // $all_heats is { roundid => array of heats } for all the roundids in
    // this round, obtained from this first query.
    $all_heats = array();
    $get_heat_stmt->execute(array(':round' => $round_level));
    foreach ($get_heat_stmt as $row) {
      $roundid = $row[0];
      $heat = $row[1];
      if (!isset($all_heats[$roundid])) {
        $all_heats[$roundid] = array();
        $weights[$roundid] = 0;
      }
      $all_heats[$roundid][] = $heat;
      ++$weights[$roundid];
    }
    if (count($weights) == 0) {
      continue;
    }
    
    // Normalize weights
    $min_count = min($weights);
    foreach ($weights as $roundid => $w) {
      $weights[$roundid] = round($w / $min_count);
    }

    $flag = true;
    while ($flag) {
      $flag = false;
      foreach ($all_heats as $roundid => &$heats) {
        $weight = $weights[$roundid];
        $heats_left = count($heats);
        for ($c = 0; $c < min($weight, $heats_left); ++$c) {
          $heat = array_shift($heats);
          $upd_stmt->execute(array(':masterheat' => $masterheat,
                                   ':roundid' => $roundid,
                                   ':heat' => $heat));
          $flag = true;
          ++$masterheat;
        }
      }
    }
  }
}
?>
