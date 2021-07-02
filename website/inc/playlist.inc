<?php
require_once('inc/scenes.inc');
require_once('inc/roster.inc');
require_once('inc/schedule_one_round.inc');
require_once('inc/autoadvance.inc');
require_once('inc/rounds.inc');

function create_playlist_table() {
  create_scene_tables();
  run_sql_script('playlist-table');
}

function first_playlist_entry() {
  try {
    return read_single_row('SELECT * FROM Playlist'
                           .' WHERE seq = (SELECT MIN(seq) FROM Playlist)',
                           array(), PDO::FETCH_ASSOC);
  } catch (PDOException $p) {
    if (is_no_such_table_exception($p)) {
      create_playlist_table();
      return array();
    } else {
      echo "SQL error code ".$p->getCode()."\n";
      echo $p;
    }
  }
}

function playlist_entry_for($roundid) {
  try {
    return read_single_row('SELECT * FROM Playlist'
                           .' INNER JOIN Rounds USING (classid, round)'
                           .' WHERE roundid = :roundid',
                           array(':roundid' => $roundid), PDO::FETCH_ASSOC);
  } catch (PDOException $p) {
    if (is_no_such_table_exception($p)) {
      create_playlist_table();
      return array();
    } else {
      echo "SQL error code ".$p->getCode()."\n";
      echo $p;
    }
  }
}

// Finds the next queue entry after the given one.
function playlist_entry_after($seq) {
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

// Returns false, and emits <failure>, if unable to activate the queue entry
function activate_playlist_entry(&$playlist_entry) {
  global $db;
  $roundid = read_single_value('SELECT roundid FROM Rounds'
                               .' WHERE classid = :classid AND round = :round',
                               array(':classid' => $playlist_entry['classid'],
                                     ':round' => $playlist_entry['round']));
  if (!$roundid) {
    $roundid = make_new_round($playlist_entry['classid'], $playlist_entry['round']);
  }
  if (read_single_value('SELECT COUNT(*) FROM Roster WHERE roundid = :roundid',
                                     array(':roundid' => $roundid)) == 0) {
    if ($playlist_entry['round'] == 1) {
      list($constituents, $rankids) =
          read_single_row('SELECT constituents, rankids FROM Classes'
                          .' WHERE classid = :classid',
                          array(':classid' => $playlist_entry['classid']));
      if (empty($rankids)) {
        $finalists = calculate_roster_from_classes(explode(',', $constituents),
                                                   $playlist_entry['bucketed'],
                                                   $playlist_entry['bucket_limit']);
      } else {
        $finalists = calculate_roster_from_subgroups(explode(',', $rankids),
                                                     $playlist_entry['bucketed'],
                                                     $playlist_entry['bucket_limit']);
      }
    } else {
      $finalists = calculate_roster_follow_on_round($playlist_entry['classid'],
                                                    $playlist_entry['bucketed'],
                                                    $plaaylist_entry['bucket_limit']);
    }
    json_out('finalists', $finalists);
    write_roster_entries($roundid, $playlist_entry['classid'], $finalists);
  }
  if ($playlist_entry['n_times_per_lane'] > 0 &&
      read_single_value('SELECT COUNT(*) FROM RaceChart'
                        .' WHERE roundid = :roundid',
                        array(':roundid' => $roundid)) == 0) {
    $nlanes = get_lane_count();
    if ($nlanes > 0) {
      schedule_one_round($roundid, $nlanes, $playlist_entry['n_times_per_lane'], /* trace */ false);
    }
  }

  $current = get_running_round();
  $current['roundid'] = $roundid;
  return advance_heat($current, 1, /* report_failure */ true);
}

/* When racing ends, at the completion of a round (all heats have been run), if
  the current head of Playlist matches the just-ending roundid, then:
  - Activate the next queue entry (generate the schedule if need be, set the current round).
  - If sceneid_at_finish, then set that, otherwise
  - if continue_racing_at_end, then set racing on (it was just turned off), otherwise
  - do nothing (leave racing off)
*/
function perform_playlist_after_round($roundid) {
  $entry = playlist_entry_for($roundid);
  $after = playlist_entry_after($entry['seq']);
  if ($after) {
    activate_playlist_entry($after);
  }
  if ($entry['sceneid_at_finish'] > 0) {
    write_raceinfo('playlist_scene', $entry['sceneid_at_finish']);
    write_raceinfo('playlist_scene_at', time() + 10);
  } else if ($entry['continue_racing']) {
    // Since we were racing before, DON'T re-apply the racing scene, as user may
    // have customized their set-up for whatever reason.
    set_racing_state(1);
  }
}
?>