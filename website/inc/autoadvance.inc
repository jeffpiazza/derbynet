<?php

require_once('inc/racing-state.inc');
require_once('inc/replay.inc');

// $current is an array as returned by get_running_round() (data.inc):
//    classid =>
//    roundid =>
//    heat =>
//    use_master_sched =>
//    now_racing =>
//
// $requested may be one of:
//    "first"
//    "next"
//    "prev"
//    "next-up"
//    a specific requested heat (combined with $roundid)
function get_next_heat(&$current, $requested) {
  global $db;

  if ($current['use_master_sched'] && !is_numeric($requested) &&
      // In timer test, ignore master heat scheduling
      $current['roundid'] != TIMER_TEST_ROUNDID) {
    if ($requested == 'next-up') {
      $stmt = $db->prepare('SELECT roundid, heat FROM RaceChart'
                           .' WHERE masterheat = (SELECT MIN(masterheat)'
                                                  .' FROM RaceChart'
                                                  .' WHERE (finishtime IS NULL AND finishplace IS NULL))');
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
                           ':delta' => ($requested == 'prev' ? -1 : 1)));
    } else {
      // TODO Don't think this ever happens
      $stmt = $db->prepare('SELECT roundid, heat FROM RaceChart'
                           .' WHERE masterheat = 1');
      $stmt->execute(array());
    }
    
    $rs = $stmt->fetch(PDO::FETCH_ASSOC);
    if ($rs !== false) {
      $result = array('roundid' => $rs['roundid'],
                      'heat' => $rs['heat']);
      $stmt->closeCursor();
      return $result;
    } else {
      return false;
    }
  } else {
    // Normal scheduling (not master scheduling)
    if ($requested == 'first') {
      $heat = 1;
    } else if ($requested == 'next') {
      $heat = $current['heat'] + 1;
    } else if ($requested == 'prev') {
      $heat = $current['heat'] - 1;
    } else {
      $heat = $requested;
    }
    return array('roundid' => $current['roundid'],
                 'heat' => $heat);
  }
}

// action.heat.select needs to handle changing rounds, in which case the
// requested heat is always 1 (or could be 'first').


// NOTE: There may not yet be a current heat/roundid/classid assigned,
// so some of these fields may be empty in $current.

// This gets called either for manual heat change
// (action.heat.select.inc) or automatic heat advancement in response
// to a timer message (action.timer-message.inc).

function advance_heat(&$current, $requested, $report_failure) {
  global $db;
  $ok = true;

  $next = get_next_heat($current, $requested);

  $ok = ($next !== false);
  if (!$ok && $report_failure) {
    echo '<failure code="nosuchheat">No '.$requested.' heat available.</failure>';
  }

  if ($ok && $next['roundid'] != TIMER_TEST_ROUNDID) {
    // Here: $next['roundid'] and $next['heat'] should have integer values that
    // identify the next (expected) heat.  It's possible the expected heat
    // doesn't exist or has already been run.

    // Check that the heat we're trying to set as current actually exists
    $ok = read_single_value('SELECT COUNT(*) FROM RaceChart'
                            .' WHERE roundid = :roundid AND heat = :heat',
                            array(':roundid' => $next['roundid'],
                                  ':heat' => $next['heat'])) > 0;
  }

  if (!$ok) {
    // If we got here, the requested heat doesn't exist.
    {
      set_racing_state(0);
    }
    if ($report_failure) {
      echo '<failure code="nosuchheat">No heat '.@$next['heat'].' scheduled for roundid '.@$next['roundid'].'</failure>';
    }
  }

  if ($ok) {  
    $roundid = $next['roundid'];
    $heat = $next['heat'];

    // If we got here and we're changing the current heat, stop
    // recording the last one.
    send_replay_CANCEL();

    write_raceinfo('RoundID', $roundid);
    write_raceinfo('Heat', $heat);

    if ($requested = 'next-up' || $roundid != @$current['roundid']) {
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
