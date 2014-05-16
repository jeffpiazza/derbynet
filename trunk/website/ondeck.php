<?php session_start(); ?>
<!DOCTYPE html>
<?php 
require_once('data.inc');
require_once('authorize.inc');
require_permission(VIEW_RACE_RESULTS_PERMISSION);

    $nlanes = get_lane_count();
    $now_running = get_running_round();
    $use_master_sched = use_master_sched();

    $high_water_rounds = high_water_rounds();
?>
<html>
<head>
<script type="text/javascript" src="jquery.js"></script>
<?php require('kiosk-page.inc'); ?>
<script type="text/javascript">
    var g_last_update_time = ""; // First refresh is for everything
    var g_high_water_resultid = <?php echo high_water_resultid(); ?>;
    var g_high_water_group = <?php echo $high_water_rounds[$use_master_sched ? 'round' : 'roundid']; ?>;
    var g_use_master_sched = <?php echo $use_master_sched ? 1 : 0; ?>;
    var g_using_groupid = true;
</script>
<script type="text/javascript" src="update.js"></script>
<title>Race Schedule</title>
<?php require('stylesheet.inc'); ?>
</head>
<body>
<?php
$banner_title = 'Racing Heats'; require('banner.inc');
running_round_header($now_running);

require_once('rounds.inc');
$groups = all_racing_groups();

$sql = 'SELECT'
  .' Classes.class, round, heat, lane, finishtime, resultid, completed, '
  .($use_master_sched ? 'round' : 'Rounds.roundid').' as racinggroup,'
  .($use_master_sched ? 'masterheat' : 'heat').' as seq,'
  .' RegistrationInfo.carnumber, RegistrationInfo.firstname, RegistrationInfo.lastname,'
  .' Classes.classid, Rounds.roundid, RaceChart.racerid'
  .' FROM Classes'
  .' INNER JOIN (Rounds'
  .' INNER JOIN (Roster'
  .' INNER JOIN (RegistrationInfo'
  .' INNER JOIN RaceChart'
  .' ON RegistrationInfo.racerid = RaceChart.racerid)'
  .' ON Roster.racerid = RegistrationInfo.Racerid)'
  .' ON Rounds.roundid = Roster.roundid)'
  .' ON Rounds.classid = Classes.classid'
  .' WHERE Rounds.roundid = RaceChart.roundid'
  .' ORDER BY '.($use_master_sched ? 'round, masterheat, lane' : 'class, round, heat, lane');
$stmt = $db->query($sql);
if ($stmt === FALSE) {
  $info = $db->errorInfo();
  echo '<h2>Error: '.$info[2].'</h2>'."\n";
 }
?>
<table class="main_table">
<?php

function byes($n) {
  $result = '';
  while ($n > 0) {
    $result .= '<td>Bye</td>';
    --$n;
  }
  return $result;
}

function write_heat_row($entry, $heat_row, $lane) {
  global $nlanes;
  global $use_master_sched;

  if ($entry) {
    $heat_row .= byes($nlanes - $lane + 1);
    $heat = $entry['Heat'];
    $heat_label = 'heat_'.$entry['RoundID'].'_'.$heat;
    $seq = $entry['Seq'];
    echo '<tr id="'.$heat_label.'" class="d'.($seq & 1).' '.$heat_label.'">'
      .'<th>'
      .($use_master_sched ? $entry['Class'].' ' : '')
      .'Heat '.$heat.'</th>'
      .$heat_row.'</tr>'."\n";
  }
}

$rs = $stmt->fetch(PDO::FETCH_ASSOC);
foreach ($groups as $group) {
  // Generate header and tbody
  $roundno = $group['round'];
  $groupid = $group['groupid'];

  echo '<tbody id="group_'.$groupid.'" class="group_'.$groupid.'">'."\n";
  echo '<tr><th/><th class="group_spacer wide" colspan="'.$nlanes.'"/></tr>'."\n";
  echo '<tr><th class="pre_group_title"/>'
      .'<th class="group_title wide" colspan="'.$nlanes.'">'.$group['groupname'].'</th>'
      .'</tr>'."\n";

  // Draw a new set of lane headers.
  // If no heats have been scheduled, then $nlanes isn't determined, and this won't 
  // produce any actual lane headers.
  echo '<tr>';
  echo '<th></th>';
  for ($l = 1; $l <= $nlanes; ++$l)
    echo '<th>Lane '.$l.'</th>';
  echo "</tr>\n";

  $seq = -1;
  $first_entry = '';
  $heat_row = '';
  while ($rs and $rs['racinggroup'] == $groupid) {
    if ($seq <> $rs['seq']) {
      write_heat_row($first_entry, $heat_row, @$lane);
      $heat_row = '';
      $seq = $rs['seq'];
	  // TODO: Make all lowercase keys
      $first_entry = array('RoundID' => $rs['roundid'],
						   'Heat' => $rs['heat'],
						   'Class' => $rs['class'],
						   'Seq' => $seq);
      $lane = 1;
    }

    // Here, $lane is one more than the lane number of the last result we've added for this
    // group/heat.  $new_lane will be the lane number of the current result.  Normally
    // $new_lane equals $lane, unless there are byes (empty lanes) in between.

    $new_lane = $rs['lane'];
    if ($new_lane) {
      $heat_row .= byes($new_lane - $lane);
      $lane = $new_lane;

      // Add the cell with the result we just got.
      // $ft = $rs['finishtime'];
      $heat_row .= '<td class="lane_'.$lane.' resultid_'.$rs['resultid'].'">'
		.'<a class="racer_link" href="racer-results.php?racerid='.$rs['racerid'].'">'
		.'<span class="car">'.$rs['carnumber'].'</span><br/>'."\n"
		.'<span class="racer">('.$rs['firstname'].' '.$rs['lastname'].')</span><br/>'."\n"
		.'<span class="time"></span>' // Javascript will fill in the times, later
		.'</a>'
		.'</td>';
      ++$lane;
    } else {
      echo '<tr>'
	.'<th class="unsched wide" colspan="'.$nlanes.'">Heats have not yet been scheduled.</th>'
	.'</tr>'."\n";
    }
    $rs = $stmt->fetch(PDO::FETCH_ASSOC);
  }

  write_heat_row($first_entry, $heat_row, @$lane);

  echo '</tbody>'."\n";
}

$stmt->closeCursor();
?>
</table>
<div id="ajax_failure" class="hidden">
  Ajax request failed with <span id="ajax_status">0</span>.
</div>
</body>
</html>