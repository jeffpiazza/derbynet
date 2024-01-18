<?php

// Normally we use max-runs-per-car = 0, so that each car runs once per lane.
// For some unusual situations, we want to limit to just 1 run per car (no
// matter which lane).  In that case, we first pad out the roster with BYEs so
// each car can get one run before we would start running anyone's second run.
// Then we form the rough schedule as a simple rotation chart (because the
// normal chart generators may put a given car only in heats that include a
// second run for some other car).  Finally, in make_ordered_schedule, we
// abandon those heats from the rough schedule that would cause any car to have
// second run.

require_once('inc/schedule_rough.inc');
require_once('inc/schedule_ordered.inc');
require_once('inc/schedule_utils.inc');
require_once('inc/schedule_rotation.inc');
require_once('inc/events.inc');

// Returns roster, where roster[sequence] = racerid,
// 0 <= sequence < number of racers in roundid.
function read_roster($roundid, $by_car_number = false) {
  global $db;
  $roster = array();
  $stmt = $db->prepare('SELECT rosterid, Roster.racerid FROM Roster'
                       .' INNER JOIN RegistrationInfo'
                       .' ON Roster.racerid = RegistrationInfo.racerid'
                       .' WHERE roundid = :roundid'
                       .' AND passedinspection = 1'
                       .($by_car_number ? ' ORDER BY carnumber, rosterid' : ' ORDER BY rosterid'));
  $stmt->execute(array(':roundid' => $roundid));
  foreach ($stmt as $row) {
    $roster[] = $row['racerid'];
  }
  return $roster;
}

// Converts a schedule (racers identified by sequence, 0 <= c < $ncars) to a
// race chart (racers identified by racerid).
function form_chart(&$schedule, &$roster) {
  $heats = array();
  foreach ($schedule as $sched_heat) {
    $heat = array();
    foreach ($sched_heat as $index) {
      $heat[] = $roster[$index];
    }
    $heats[] = $heat;
  }
  return $heats;
}

// Writes the race chart entries into the database.
function write_chart($roundid, &$heats) {
  global $db;
  record_event(EVENT_SCHEDULE_ADDED, array('roundid' => $roundid));
  $db->exec('DELETE FROM RaceChart WHERE roundid = '.$roundid);
  $classid = read_single_value('SELECT classid FROM Rounds WHERE roundid = '.$roundid);
  $stmt = $db->prepare('INSERT INTO RaceChart(roundid, heat, lane, racerid, classid)'
                       .' VALUES(:roundid, :heat, :lane, :racerid, :classid)');
  $heat_index = 0;
  foreach ($heats as $heat) {
    ++$heat_index;
    $lane_index = 0;
    foreach ($heat as $racerid) {
      ++$lane_index;
      if ($racerid) {
        $stmt->execute(array(':roundid' => $roundid,
                             ':heat' => $heat_index,
                             ':lane' => $lane_index,
                             ':racerid' => $racerid,
                             ':classid' => $classid));
      }
    }
  }
}

// The top-level scheduling function.  Constructs a schedule, converts to a race
// chart, and writes the race chart into the database.
function schedule_one_round($roundid, $nlanes, $n_times_per_lane, $trace) {
  $rotation_schedule = read_raceinfo('rotation-schedule', 0);
  $roster = read_roster($roundid, /*by_car_number=*/$rotation_schedule);
  if (count($roster) < 2) {
    return false;
  }
  while (count($roster) < $nlanes) {
    $roster[] = null;  // Byes to bring up the roster count to at least the number of lanes
  }
  $max_runs_per_car = read_raceinfo('max-runs-per-car', 0);
  while (count($roster) * $max_runs_per_car % $nlanes != 0) {
    $roster[] = null;
  }
  if (!$rotation_schedule) {
    $sched = make_ordered_schedule($nlanes, count($roster), $n_times_per_lane,
                                   $max_runs_per_car, $trace);
  } else {
    $sched = make_rotation_schedule($nlanes, count($roster), $n_times_per_lane, $trace);
  }
  write_chart($roundid, spread_across_unused_lanes(form_chart($sched, $roster)));
  return true;
}

?>
