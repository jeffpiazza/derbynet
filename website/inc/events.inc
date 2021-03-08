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

// Not yet implemented:
define('EVENT_PLAYLIST_ADDED', 209);
define('EVENT_PLAYLIST_REMOVED', 210);

define('EVENT_CLASS_DELETED', 301);
define('EVENT_RANK_DELETED', 302);
define('EVENT_ROUND_DELETED', 303);
define('EVENT_SCHEDULE_DELETED', 303);


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

function record_event($action, $racerid, $other = '') {
  record_event_sql('INSERT INTO Events(tstamp, action, racerid, other)'
                   .' VALUES('.dbnow_expr().', :action, :racerid, :other)',
                   array(':action' => $action,
                         ':racerid' => $racerid,
                         ':other' => $other));
}

function record_event_create_class($classid, $classname) {
  record_event_sql('INSERT INTO Events(tstamp, action, classid, other)'
                   .' VALUES('.dbnow_expr().', :action, :classid, :other)',
                   array(':action' => EVENT_CLASS_ADDED,
                         ':classid' => $classid,
                         ':other' => $classname));
}
function record_event_delete_class($classid) {
  record_event_sql('INSERT INTO Events(tstamp, action, classid)'
                   .' VALUES('.dbnow_expr().', :action, :classid)',
                   array(':action' => EVENT_CLASS_DELETED,
                         ':classid' => $classid));
}
function record_event_create_rank($classid, $rankid, $rankname) {
  record_event_sql('INSERT INTO Events(tstamp, action, classid, rankid, other)'
                   .' VALUES('.dbnow_expr().', :action, :classid, :rankid, :other)',
                   array(':action' => EVENT_RANK_ADDED,
                         ':classid' => $classid,
                         ':rankid' => $rankid,
                         ':other' => $rankname));
}
// Either classid or rankid can be -1 for unknown
function record_event_delete_rank($classid, $rankid) {
  record_event_sql('INSERT INTO Events(tstamp, action, classid, rankid)'
                   .' VALUES('.dbnow_expr().', :action, :classid, :rankid)',
                   array(':action' => EVENT_RANK_DELETED,
                         ':classid' => $classid,
                         ':rankid' => $rankid));
}
// $roundid can be -1 for not known
function record_event_create_round($roundid, $classid, $roundno) {
  if ($roundid < 0) {
    $roundid = read_single_value('SELECT roundid FROM Rounds'
                                 .' WHERE classid = :classid AND round = :roundno',
                                 array(':classid' => $classid,
                                       ':roundno' => $roundno));
  }
  record_event_sql('INSERT INTO Events(tstamp, action, classid, roundid, other)'
                   .' VALUES('.dbnow_expr().', :action, :classid, :roundid, :other)',
                   array(':action' => EVENT_ROUND_ADDED,
                         ':classid' => $classid,
                         ':roundid' => $roundid,
                         ':other' => 'round '.$roundno));
}

function record_event_delete_round($classid, $roundid) {
  $roundno = -1;
  if ($roundid >= 0) {
    $round = read_single_row('SELECT classid, round FROM Rounds'
                             .' WHERE roundid = :roundid',
                             array(':roundid' => $roundid));
    $classid = $round[0];
    $roundno = $round[1];
  }
  $classname = read_single_value('SELECT class FROM Classes WHERE classid = :classid',
                                 array(':classid' => $classid));
  if ($classname === false) {
    $classname = '(No such class)';
  }

  record_event_sql('INSERT INTO Events(tstamp, action, roundid, classid, other)'
                   .' VALUE('.dbnow_expr().', :action, :roundid, :classid, :other)',
                   array(':action' => EVENT_ROUND_DELETED,
                         ':roundid' => $roundid,
                         ':classid' => $classid,
                         ':other' => "$classname, Round $roundno"));
}
function record_event_create_schedule($roundid) {
  record_event_sql('INSERT INTO Events(tstamp, action, roundid)'
                   .' VALUES('.dbnow_expr().', :action, :roundid)',
                   array(':action' => EVENT_SCHEDULE_ADDED,
                         ':roundid' => $roundid));
}
function record_event_delete_schedule($roundid, $other) {
  record_event_sql('INSERT INTO Events(tstamp, action, roundid, other)'
                   .' VALUES('.dbnow_expr().', :action, :roundid, :other)',
                   array(':action' => EVENT_SCHEDULE_DELETED,
                         ':roundid' => $roundid,
                         ':other' => $other));
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

function record_event_purge($action) {
  record_event_sql('INSERT INTO Events(tstamp, action) VALUES('.dbnow_expr().', :action)',
                   array(':action' => $action));
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