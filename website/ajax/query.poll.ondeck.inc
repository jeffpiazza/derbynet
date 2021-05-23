<?php
require_once('inc/json-current-heat.inc');
require_once('inc/autoadvance.inc');
require_once('inc/ordinals.inc');

// This script produces XML in response to a request from a page that wants to 
// update itself as the race progresses.  In particular, it supports ondeck.php (showing 
// race results by heat, as well as the schedule for future heats) and racer-results.php
// (showing race results organized by racer).
//
// The updates that each of these pages has to track comes in different flavors:
//
// (1) The heat that's "current" advances.
// (2) New heat results (race times) have been recorded since the last update.
// (3) New heats have been scheduled for a racing round that we already knew about.
//     (That is, there was an entry in the Rounds table, even if the heats hadn't 
//     yet been scheduled.)
// (4) New racing rounds have been scheduled (new entries in Rounds table).
//
// Deletions are also possible, but relatively rare.  We don't worry about these, and
// instead rely on updates being triggered by new, replacement data.
//
// This script returns an XML document that looks like:
//
// <ondeck>
//   <current-heat classid="{classid}"
//                 roundid="{roundid}"
//                 round="{round}"
//                 tbodyid="{tbodyid}"
//                 heat="{heat number}"
//     >{class name}</class>
//   <lanes n="{nlanes}"/>
//   <options update-period="{period in ms}" use-master-sched="(0 or 1)"/>
//
//   <update resultid="{resultid}" result="{finishtime, as n.nnn}"/>
//   <update resultid="{resultid}" result="{finishtime, as n.nnn}"/>
//   ...
//
//   <has_new_schedule tbodyid="{tbodyid}"/>
//   ...
//
//   <high_water roundid= round= tbodyid= resultid= completed= />
// </ondeck>
//
// The class, round, and heat elements identify the "current" heat.
//
// The update elements provide heat times for heats that completed after the page's last update.
// The client-side page, via ondeck-update.js, passes a date/time parameter named 'since' to identify
// the high water mark of Completed values that it's already displaying.
//
// The most_recent element tells the page what its new high water mark for Completed values is.
//
// The final_round element gives a high water mark for RoundIDs.  When this value changes, relative
// to what the page already knows about, it gives up on incrementality and just reloads the whole page.

// date/time string of last update time (high-water completed)
// Format: '2013-11-14 14:57:14'
$since = @$_GET['since'];
$hwresultid = @$_GET['high_water_resultid'];
if (!$hwresultid) {
  $hwresultid = 0;
}

// If true, the client is using master scheduling and therefore merging all the
// rounds with the same round number.  We identify everything by round number in
// that case, instead of roundid.
$merge_rounds = @$_GET['merge_rounds'];
$time_format = get_finishtime_formatting_string();

$use_master_sched = use_master_sched();
$now_running = get_running_round();
json_out('current-heat', json_current_heat($now_running, $use_master_sched));

$next = get_next_heat($now_running, 'next');
if ($next !== false) {
  json_out('next-heat', array('roundid' => $next['roundid'],
                              'heat' => $next['heat']));
}

$prev = get_next_heat($now_running, 'prev');
if ($prev !== false) {
  json_out('prev-heat', array('roundid' => $prev['roundid'],
                              'heat' => $prev['heat']));
}
json_out('lanes', get_lane_count_from_results());
json_out('options', array('update-period' => update_period(),
                          'use-master-sched' => $use_master_sched));

$sql = 'SELECT'
.' lane, resultid, finishtime, finishplace, completed'
.' FROM Rounds'
.' INNER JOIN RaceChart'
.' ON RaceChart.roundid = Rounds.roundid'
.' WHERE (finishtime IS NOT NULL OR finishplace IS NOT NULL)'
.($since
  ? ' AND completed > '.db_compare_completed(':since')
  : "") 
.' ORDER BY completed, resultid, lane';
$stmt = $db->prepare($sql);
$stmt->execute($since ? array(':since' => $since) : array());

// <update> elements for heats recorded since the ['since']
$updates = array();
$max_time = $since;
$use_points = read_raceinfo_boolean('use-points');
foreach ($stmt as $rs) {
  if ($use_points) {
    $val = ordinal($rs['finishplace']);
  } else {
    $val = sprintf($time_format, $rs['finishtime']);
  }
  $updates[] = array('resultid' => $rs['resultid'],
                     'result' => $val);
  if ($rs['completed'] > $max_time) {
    $max_time = $rs['completed'];
  }
}
json_out('updates', $updates);

// <has_new_schedule> elements for rounds affected by newly-entered racechart rows
$new_rounds = array();

$sql = 'SELECT DISTINCT Rounds.roundid, round'
.' FROM RaceChart'
.' INNER JOIN Rounds'
.' ON RaceChart.roundid = Rounds.roundid'
.' WHERE resultid > '.$hwresultid
.' ORDER BY Rounds.roundid';
$stmt = $db->query($sql);
if ($stmt === FALSE) {
  $info = $db->errorInfo();
  echo '<error msg="'.$info[2].'" query="'.$sql.'"/>'."\n";
}

foreach ($stmt as $rs) {
  $new_rounds[] = $rs[$merge_rounds ? 'round' : 'roundid'];
}
json_out('new-rounds', $new_rounds);

$high_water_rounds = high_water_rounds();
json_out('high-water', array('roundid' => $high_water_rounds['roundid'],
                             'round' => $high_water_rounds['round'],
                             'tbodyid' => $high_water_rounds[$merge_rounds ? 'round' : 'roundid'],
                             'resultid' => high_water_resultid(),
                             'completed' => $max_time));
?>