<?php
require_once('inc/scenes.inc');
require_once('inc/schedule_one_round.inc');

function playlist_entry_for($roundid) {
  return read_single_row('SELECT * FROM Playlist WHERE roundid = :roundid',
                         array(':roundid' => $roundid), PDO::FETCH_ASSOC);
}

// Finds the next playlist entry after the given one.
function playlist_after($seq) {
  return read_single_row('SELECT * FROM Playlist'
                         .' WHERE seq = (SELECT MIN(seq) FROM Playlist'
                         .'              WHERE seq > :seq)',
                         array(':seq' => $seq), PDO::FETCH_ASSOC);
}

function round_is_completed($roundid) {
  return read_single_value('SELECT COUNT(*) FROM RaceChart'
                           .' WHERE roundid = :roundid'
                           .'  AND (completed IS NULL OR completed = \'\')',
                           array(':roundid' => $roundid)) == 0;
}

// Returns false, and emits <failure>, if unable to activate the playlist entry
function activate_playlist(&$playlist) {
  $roundid = $playlist['roundid'];
  if ($playlist['n_times_per_lane'] > 0 &&
      read_single_value('SELECT COUNT(*) FROM RaceChart'
                        .' WHERE roundid = :roundid',
                        array(':roundid' => $roundid)) == 0) {
    $nlanes = get_lane_count();
    if ($nlanes > 0) {
      schedule_one_round($roundid, $nlanes, $playlist['n_times_per_lane'], /* trace */ false);
    }
  }

  $current = get_running_round();
  $current['roundid'] = $roundid;
  return advance_heat($current, 1, /* report_failure */ true);
}

/* When racing ends, at the completion of a round (all heats have been run), if
  the current head of Playlist matches the just-ending roundid, then:
  - Activate the next playlist entry (generate the schedule if need be, set the current round).
  - If sceneid_at_finish, then set that, otherwise
  - if continue_racing_at_end, then set racing on (it was just turned off), otherwise
  - do nothing (leave racing off)
*/
function playlist_after_round($roundid) {
  $playlist = playlist_entry_for($roundid);
  $after = playlist_after($playlist['seq']);
  if ($after) {
    activate_playlist($after);
  }
  if ($playlist['sceneid_at_finish'] > 0) {
    // TODO We want to queue a new scene for a short time after racing ends, not
    // apply the scene immediately.
    apply_scene($playlist['sceneid_at_finish']);
  } else if ($playlist['continue_racing']) {
    // Since we were racing before, DON'T re-apply the racing scene, as user may
    // have customized their set-up for whatever reason.
    set_racing_state(1);
  }
}
?>