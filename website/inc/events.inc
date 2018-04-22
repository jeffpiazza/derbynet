<?php

require_once('inc/schema_version.inc');
require_once('inc/sql-script.inc');

// The 'Events' table provides a history of important changes in the data.
define('EVENT_CHECKIN', 1);
define('EVENT_RACER_PHOTO_ASSIGN', 2);
define('EVENT_CAR_PHOTO_ASSIGN', 3);

function create_events_table() {
  run_sql_script('events-table');
}

function record_event($action, $racerid, $other = '') {
  global $db;

  do {
    $retry = false;
    try {
      $stmt = $db->prepare('INSERT INTO Events(tstamp, action, racerid, other)'
                           .' VALUES('.dbnow_expr().', :action, :racerid, :other)');
      $stmt->execute(array(':action' => $action,
                           ':racerid' => $racerid,
                           ':other' => $other));
    } catch (PDOException $p) {
      if ($p->getCode() == '42S02') {
        create_events_table();
        $retry = true;
      } else {
        echo $p;
      }
    }
  } while ($retry);
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
    if ($p->getCode() == '42S02') {
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