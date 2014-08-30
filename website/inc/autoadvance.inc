<?php

require_once('inc/racing-state.inc');
require_once('inc/replay.inc');

// NOTE: There may not yet be a current heat/roundid/classid assigned,
// so some of these fields may be empty in $current.

// This gets called either for manual heat change
// (action.select-heat.inc) or automatic heat advancement in response
// to a timer message (action.timer-message.inc).
function advance_heat(&$current, $roundid, $requested_heat, $echo_failure) {
  global $db;
  $ok = true;

  if ($current['use_master_sched'] && 
      ($requested_heat == 'first' || $requested_heat == 'next' || $requested_heat == 'prev')) {
    if ($requested_heat == 'first') {
      $stmt = $db->prepare('SELECT roundid, heat FROM RaceChart'
                           .' WHERE masterheat = (SELECT MIN(masterheat)'
                                                  .' FROM RaceChart'
                                                  .' WHERE finishtime IS NULL)');
      $stmt->execute(array());
    } else if (isset($current['roundid']) && isset($current['heat'])) {
      $stmt = $db->prepare('SELECT roundid, heat FROM RaceChart'
                           .' WHERE masterheat = '
                           .'   (SELECT DISTINCT masterheat FROM RaceChart'
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
    if ($rs !== false) {
      $roundid = $rs['roundid'];
      $heat = $rs['heat'];
      $stmt->closeCursor();
    } else {
      $ok = false;
    }
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

  // Here: $roundid and $heat should have integer values that identify
  // the next (expected) heat.  It's possible the expected heat
  // doesn't exist or has already been run.

  // Check that the heat we're trying to set as current actually exists
  if ($ok) {
    $ok = read_single_value('SELECT COUNT(*) FROM RaceChart'
                            .' WHERE roundid = :roundid AND heat = :heat',
                            array(':roundid' => $roundid,
                                  ':heat' => $heat)) > 0;
  }

  if (!$ok) {
    // If we got here, the requested heat doesn't exist.
    // if ($requested_heat == 'next' || $requested_heat == 'prev') 
    {
      set_racing_state(0);
    }
    if ($echo_failure) {
      echo '<failure code="nosuchheat">No heat '.$heat.' scheduled for roundid '.$roundid.'</failure>';
    }
  } else {
    // If we got here and we're changing the current heat, stop
    // recording the last one.
    send_replay_CANCEL();

    write_raceinfo('RoundID', $roundid);
    write_raceinfo('Heat', $heat);
    if ($roundid != @$current['roundid']) {
      write_raceinfo('ClassID', read_single_value('SELECT classid FROM Rounds WHERE roundid = :roundid',
                                                  array(':roundid' => $roundid)));
    }

    // Advancing the heat while in racing mode may have additional consequences
    if ($current['now_racing']) {
      $current['roundid'] = $roundid;
      $current['heat'] = $heat;
      if (!racing_state_allowed($current)) {
        // End of a group.  set_racing_state(0) will send a CANCEL
        set_racing_state(0);
      } else {
        // Starting a new heat, roll camera
        send_replay_START($current);
      }
    }
  }

  return $ok;
}

?>
