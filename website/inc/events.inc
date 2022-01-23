<?php

require_once('inc/schema_version.inc');
require_once('inc/sql-script.inc');
require_once('inc/ordinals.inc');

// The 'Events' table provides a history of important changes in the data.
// All entries populate seq, tstamp, and action fields.
// Other fields are populated according to the action:

// A racer was checked in (racerid)
define('EVENT_CHECKIN', 1);
// A racer's photo was assigned (racerid)
define('EVENT_RACER_PHOTO_ASSIGN', 2);
// A racer's car photo was assigned (racerid)
define('EVENT_CAR_PHOTO_ASSIGN', 3);

define('EVENT_TIMER_MALFUNCTION', 20);
// A 'FINISHED' message arrived when not in a racing state (
define('EVENT_TIMER_RESULT_REFUSED', 25);

define('EVENT_PURGE_RESULTS', 90);
define('EVENT_PURGE_SCHEDULES', 91);
define('EVENT_PURGE_RACERS', 92);
define('EVENT_PURGE_AWARDS', 93);
define('EVENT_PURGE_RESULTS_ONE_ROUND', 94);

// A racer's heat result was discarded (we record the value that was discarded,
// in case questions arise later).  (racerid, roundid, heat, lane, other)
define('EVENT_RESULT_DISCARDED', 100);  // racerid, roundid, heat, lane, other(place)

// RaceChart has its own 'completed' timestamp field, and usually that's enough.
// Only if we discard results do we add a HEAT_COMPLETED event, after the fact,
// for the heat.  The HEAT_COMPLETED event captures roundid, heat, and tstamp
// only, the tstamp being for the previous value of the RaceChart 'completed'
// field.
define('EVENT_HEAT_COMPLETED', 101);  // roundid, heat
define('EVENT_HEAT_MANUALLY_ENTERED', 102);

define('EVENT_CLASS_ADDED', 200);
define('EVENT_RANK_ADDED', 201);
define('EVENT_ROUND_ADDED', 202);
define('EVENT_SCHEDULE_ADDED', 203);
define('EVENT_PARTITION_ADDED', 204);

// Not yet implemented:
define('EVENT_PLAYLIST_ADDED', 209);
define('EVENT_PLAYLIST_REMOVED', 210);

define('EVENT_CLASS_DELETED', 301);
// Either classid or rankid can be -1 for unknown
define('EVENT_RANK_DELETED', 302);
define('EVENT_ROUND_DELETED', 303);
define('EVENT_SCHEDULE_DELETED', 303);
define('EVENT_PARTITION_DELETED', 304);

define('EVENT_GROUP_FORMATION_RULE_APPLIED', 401);
define('EVENT_PARTITION_MOVED', 402);
define('EVENT_PARTITION_RENAMED', 403);

define('EVENT_ACTIONS_TABLE',
array(
  'award.adhoc', 'award.delete', 'award.edit', 'award.import',
  'award.order', 'award.present', 'award.winner', 'award.xbs',
  'ballot.open',
  'class.add', 'class.delete', 'class.edit', 'class.order',
  'database.execute', 'database.purge',
  'file.directory.new.nodata',
  'heat.reinstate', 'heat.rerun', 'heat.select',
  'kiosk.assign', 'partition.add', 'partition.apply-rule',
  'partition.delete', 'partition.edit', 'partition.move', 'partition.order',
  'photo.assign', 'photo.crop', 'photo.delete', 'photo.rotate', 'photo.upload',
  'playlist.add', 'playlist.delete', 'playlist.edit', 'playlist.order', 'playlist.start',
  'racer.add', 'racer.bulk', 'racer.delete', 'racer.edit', 'racer.fake', 'racer.import', 'racer.pass',
  'rank.add', 'rank.delete', 'rank.edit', 'rank.order',
  'replay.message', 'replay.test', 'replay.trigger',
  'result.delete', 'result.import', 'result.write',
  'roster.delete', 'roster.new',
  'scene.add', 'scene.apply', 'scene.delete', 'scene.setkiosk',
  'schedule.generate', 'schedule.reschedule', 'schedule.unschedule',
  'settings.write', 'setup.nodata',
  // 'action.timer-message',
  'timer.assign-device', 'timer.assign-flag', 'timer.assign-port', 'timer.remote-start', 'timer.test',
  'video.upload',
  'vote.cast',
));

function create_events_table() {
  run_sql_script('events-table');
}

function record_event_sql($sql, $params) {
  global $db;

  do {
    $retry = false;
    try {
      $stmt = $db->prepare($sql);
      $stmt->execute($params);
    } catch (PDOException $p) {
      if (is_no_such_table_exception($p)) {
        create_events_table();
        $retry = true;
      } else {
        echo $p;
      }
    }
  } while ($retry);
}

function record_event($action, $fields = []) {
  $sql1 = 'INSERT INTO Events(tstamp, action';
  $sql2 = ' VALUES('.dbnow_expr().', :action';
  $params = array(':action' => $action);

  foreach ($fields as $f => $val) {
    $sql1 .= ', '.$f;
    $sql2 .= ', :'.$f;
    $params[':'.$f] = $val;
  }
  
  $sql1 .= ')';
  $sql2 .= ')';
  record_event_sql($sql1.$sql2, $params);
}

// Call this BEFORE rewriting a heat result
// $where is a WHERE clause against RaceChart for results being discarded.
function record_event_for_discarded_heat($where, $params) {
  global $db;
  $stmt = $db->prepare('SELECT racerid, roundid, heat, lane, finishtime, finishplace'
                       .' FROM RaceChart '.$where);
  $stmt->execute($params);
  foreach ($stmt as $old_result) {
    record_event_sql('INSERT INTO Events(tstamp, action, racerid, roundid, heat, lane, other)'
                     .' VALUES('.dbnow_expr().', :action, :racerid, :roundid, :heat, :lane, :other)',
                     array(':action' => EVENT_RESULT_DISCARDED,
                           ':racerid' => $old_result['racerid'],
                           ':roundid' => $old_result['roundid'],
                           ':heat' => $old_result['heat'],
                           ':lane' => $old_result['lane'],
                           ':other' => $old_result['finishtime']
                           .' ('.ordinal($old_result['finishplace']).')'));
  }
}

// $results maps lane to {time, place}
function record_event_for_refused_result($results) {
  foreach ($results as $lane => $r) {
    if ($r) {
      $other = '';
      if (isset($r['time'])) {
        $other .= $r['time'];
      }
      if (isset($r['place'])) {
        $other .= ' ('.ordinal($r['place']).')';
      }
      record_event_sql('INSERT INTO Events(tstamp, action, lane, other)'
                       .' VALUES('.dbnow_expr().', :action, :lane, :other)',
                       array(':action' => EVENT_TIMER_RESULT_REFUSED,
                             ':lane' => $lane + 1,
                             ':other' => $other));
    }
  }
}

// Returns an array of racer records, in the order in which they had the
// specified event action recorded
function racers_with_actions($action) {
  global $db;

  $result = array();
  try {
    $stmt = $db->prepare('SELECT seq, RegistrationInfo.racerid, carnumber, lastname, firstname, carname, '
                         .' RegistrationInfo.classid, class, RegistrationInfo.rankid, rank, imagefile'
                         .' FROM '.inner_join('Events',
                                              'RegistrationInfo',
                                              'Events.racerid = RegistrationInfo.racerid',
                                              'Classes',
                                              'RegistrationInfo.classid = Classes.classid',
                                              'Ranks',
                                              'RegistrationInfo.rankid = Ranks.rankid')
                         .' WHERE action = :action ORDER BY Events.seq DESC');
    $stmt->execute(array(':action' => $action));
    foreach ($stmt as $row) {
      $result[] = array('seq' => $row['seq'],
                        'racerid' => $row['racerid'],
                        'carnumber' => $row['carnumber'],
                        'lastname' => $row['lastname'],
                        'firstname' => $row['firstname'],
                        'carname' => $row['carname'],
                        'classid' => $row['classid'],
                        'class' => $row['class'],
                        'rankid' => $row['rankid'],
                        'rank' => $row['rank'],
                        'imagefile' => $row['imagefile']);
    }
  } catch (PDOException $p) {
    if (is_no_such_table_exception($p)) {
      // Silently fail -- no events table implies no events
    } else {
      echo $p;
    }
  }
  return $result;
}

// Returns an array of racer records, in the order in which they had racer photos assigned
function racers_with_photos() {
  return racers_with_actions(EVENT_RACER_PHOTO_ASSIGN);
}

?>