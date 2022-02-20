<?php

require_once('inc/rounds.inc');
require_once('inc/playlist.inc');

function json_all_rounds(&$now_running) {
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
      'roundid' => $roundid,
      'classid' => $round['classid'],
      'class' => $round['class'],
      'round' => $round['round'],
      'aggregate' => $round['aggregate'] ? true : false,
      'roster_size' => $round['roster_size'],
      'passed' => $round['passed'],
      'unscheduled' => $round['unscheduled'],
      'heats_scheduled' => $round['heats_scheduled'],
      'heats_run' => $round['heats_run'],
      'name' => $round['roundname']);
    if ($roundid == $next_roundid) {
      $r['next-round'] = true;
    }
    $result[] = $r;
  }
  return $result;
}

?>