<?php

// To control the timer, and optionally the replay application, we
// keep a NowRacingState setting (value 0 or 1) in the database.
//
// NowRacingState 0 aborts any pending timer activity, aborts the
// replay app's recording and refuses any heat results sent from the
// timer.
//
// NowRacingState 1 implements a proper "hands-free racing" mode: upon
// the conclusion of one heat, the next heat becomes current, the
// timer and replay applications are primed for the next heat.  Any
// incoming timer results in this state are recorded, and trigger the
// replay application, before advancing to the next heat.
//
// If an auto heat advance fails (because we just ran the last heat in
// the round), NowRacingState turns off.  Additionally, manually
// changing the current heat turns off NowRacingState.
//
// An attempt to re-enable NowRacingState will be refused if the
// current round has recorded finish times; the operator will have to
// choose "re-run heat" to discard them before NowRacingState can be
// resumed.
//
// NowRacingState is included as the ['now_racing'] element in the
// array returned by get_running_round().

require_once('inc/replay.inc');


function racing_state_allowed(&$current) {
  if (!isset($current['roundid']) || !isset($current['heat'])) {
    return false;
  }
  if ($current['roundid'] == TIMER_TEST_ROUNDID) {
    return true;
  }
  $finished = read_single_value('SELECT COUNT(*) FROM RaceChart'
                                .' WHERE roundid = :roundid'
                                .'  AND  heat = :heat'
                                .'  AND  (finishtime IS NOT NULL OR finishplace IS NOT NULL)',
                                array(':roundid' => $current['roundid'],
                                      ':heat' => $current['heat']));
  if ($finished > 0) {
    return false;
  }
  $ready = read_single_value('SELECT COUNT(*) FROM RaceChart'
                             .' WHERE roundid = :roundid'
                             .'  AND  heat = :heat'
                             .'  AND  finishtime IS NULL AND finishplace IS NULL',
                             array(':roundid' => $current['roundid'],
                                   ':heat' => $current['heat']));
  if ($ready == 0) {
    return false;
  }

  return true;
}

function get_racing_state() {
  return read_raceinfo('NowRacingState', 0) != 0;
}

// Returns FALSE if an attempt to turn on NowRacingState is refused.
// Turning off NowRacingState always succeeds.
function set_racing_state($state) {
  $state = ($state ? 1 : 0);

  $current = get_running_round();
  if ($state) {
    if (!racing_state_allowed($current)) {
      return false;
    }

    if (!$current['now_racing']) {
      // This starts the video when the "Now Racing" switch turns
      // on.
      send_replay_START($current);
    }
  } else {
    if ($current['now_racing']) {
      send_replay_CANCEL();
    }
  }

  write_raceinfo('NowRacingState', $state);
  write_raceinfo('refused-results', 0);

  return true;
}

function cancel_running_round() {
  set_racing_state(0);
  write_raceinfo('RoundID', -1);
  write_raceinfo('ClassID', -1);
  write_raceinfo('Heat', -1);
}

function maybe_cancel_running_round($roundid) {
  $now_running = get_running_round();
  if ($roundid == $now_running['roundid']) {
    cancel_running_round();
  }
}

?>
