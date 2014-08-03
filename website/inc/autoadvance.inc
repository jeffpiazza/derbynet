<?php

// NOTE: There may not yet be a current heat/roundid/classid assigned,
// so some of these fields may be empty in $current.
function advance_heat(&$current, $roundid, $requested_heat, $echo_failure) {
  global $db;
  if ($current['use_master_sched'] && ($requested_heat == 'next' ||
                                       $requested_heat == 'prev')) {
    if (isset($current['roundid']) && isset($current['heat'])) {
      $stmt = $db->prepare('SELECT roundid, heat FROM RaceChart'
                           .' WHERE masterheat = '
                           .'   (SELECT masterheat FROM RaceChart'
                           .'           WHERE roundid = :roundid'
                           .'            AND  heat = :heat)'
                           .'  + :delta');
      $stmt->execute(array(':roundid' => $current['roundid'],
                           ':heat' => $current['heat'],
                           ':delta' => ($requested_heat == 'prev' ? -1 : 1)));
    } else {
      $stmt = $db->prepare('SELECT roundid, heat FROM RaceChart'
                           .' WHERE masterheat = 1');
      $stmt->execute(array());
    }
    $rs = $stmt->fetch(PDO::FETCH_ASSOC);
    $roundid = $rs['roundid'];
    $heat = $rs['heat'];
    $rs->closeCursor();
  } else {
    if (!isset($current['heat']) || $current['heat'] < 0) {
      $heat = 1;
    } else if ($requested_heat == 'next') {
      $heat = $current['heat'] + 1;
    } else if ($requested_heat == 'prev') {
      $heat = $current['heat'] - 1;
    } else {
      $heat = $requested_heat;
    }
  }

  // Check that the heat we're trying to set as current actually exists
  $ok = read_single_value('SELECT COUNT(*) FROM RaceChart'
                          .' WHERE roundid = :roundid AND heat = :heat',
                          array(':roundid' => $roundid,
                                ':heat' => $heat)) > 0;

  // TODO: Either CANCEL or REPLAY command to replay here.
  if (!$ok) {
    if ($requested_heat == 'next' || $requested_heat == 'prev') {
      write_raceinfo('NowRacingState', 0);
    }
    if ($echo_failure) {
      echo '<failure code="nosuchheat">No heat '.$heat.' scheduled for roundid '.$roundid.'</failure>';
    }
  } else {
    // TODO: START command to replay
    write_raceinfo('RoundID', $roundid);
    write_raceinfo('Heat', $heat);
    if ($roundid != @$current['roundid']) {
      write_raceinfo('ClassID', read_single_value('SELECT classid FROM Rounds WHERE roundid = :roundid',
                                                  array(':roundid' => $roundid)));
    }
  }

  return $ok;
}

?>