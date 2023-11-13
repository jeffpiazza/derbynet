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
// In-browser timer interface may get starved for events by the browser,
// resulting in not getting a chance to send events to the server as often
// as expected.
define('EVENT_TIMER_OVERDUE', 30);

define('EVENT_PURGE_RESULTS', 90);
define('EVENT_PURGE_SCHEDULES', 91);
define('EVENT_PURGE_RACERS', 92);
define('EVENT_PURGE_AWARDS', 93);
define('EVENT_PURGE_RESULTS_ONE_ROUND', 94);

// A heat result was discarded.  (roundid, heat)
define('EVENT_HEAT_RESULT_DISCARDED', 100);  // roundid, heat

define('EVENT_HEAT_RESULT_FROM_TIMER', 101);  // roundid, heat
define('EVENT_HEAT_MANUALLY_ENTERED', 102);
define('EVENT_HEAT_RESULT_REINSTATED', 103);

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


function create_events_table() {
  run_sql_script('events-table');
}

// Old-style event recording code.  Leaving the calls in place but making them
// no-ops.
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
  return;
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

function record_event_to_limit($action, $max_events, $fields = []) {
  return;
  if (read_single_value('SELECT COUNT(*) FROM Events WHERE action = :action',
                        array(':action' => $action)) < $max_events) {
    record_event($action, $fields);
  }
}

?>
