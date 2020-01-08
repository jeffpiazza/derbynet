<?php

require_once('inc/schema_version.inc');
require_once('inc/sql-script.inc');

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

// Call this BEFORE rewriting a heat result
function record_event_for_discarded_result($resultid) {
  $old_result = read_single_row('SELECT racerid, roundid, heat, lane, finishtime, finishplace'
                                .' FROM RaceChart'
                                .' WHERE resultid = :resultid',
                                array(':resultid' => $resultid),
                                PDO::FETCH_ASSOC);
  if ($old_result !== false) {
    record_event_sql('INSERT INTO Events(tstamp, action, racerid, roundid, heat, lane, other)'
                     .' VALUES('.dbnow_expr().', :action, :racerid, :roundid, :heat, :lane, :other)',
                     array(':action' => EVENT_RESULT_DISCARDED,
                           ':racerid' => $old_result['racerid'],
                           ':roundid' => $old_result['roundid'],
                           ':heat' => $old_result['heat'],
                           ':lane' => $old_result['lane'],
                           ':other' => $old_result['finishtime'].' ('.$old_result['finishplace'].')'));
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