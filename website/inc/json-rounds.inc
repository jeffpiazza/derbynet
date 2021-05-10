<?php

require_once('inc/rounds.inc');

function json_all_rounds() {
  $next_roundid = false;
  if (isset($now_running['roundid'])) {
    $q = playlist_entry_for($now_running['roundid']);
    if ($q) {
      $q = playlist_entry_after($q['seq']);
    } else {
      $q = first_playlist_entry();
    }
    if ($q) {
      $next_roundid = find_roundid($q['classid'], $q['round']);
    }
  }

  $rounds = all_rounds_with_counts();

  $result = array();
  foreach ($rounds as $round) {
    $roundid = $round['roundid'];
    $r = array(
      'roundid' => $roundid + 0,
      'classid' => $round['classid'] + 0,
      'class' => $round['class'],
      'round' => $round['round'] + 0,
      'aggregate' => $round['aggregate'] ? true : false,
      'roster_size' => $round['roster_size'] + 0,
      'passed' => $round['passed'] + 0,
      'unscheduled' => $round['unscheduled'] + 0,
      'heats_scheduled' => $round['heats_scheduled'] + 0,
      'heats_run' => $round['heats_run'] + 0,
      'name' => $round['roundname']);
    if ($roundid['roundid'] == $next_roundid) {
      $r['next-round'] = true;
    }
    $result[] = $r;
  }
  return $result;
}

?>