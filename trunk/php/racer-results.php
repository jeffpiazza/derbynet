<?php session_start(); ?>
<!DOCTYPE html>
<?php
    require_once('data.inc');

    $high_water_rounds = high_water_rounds();
?>
<html>
<head>
<script type="text/javascript" src="jquery.js"></script>
<script type="text/javascript">
    var g_last_update_time = ""; // First refresh is for everything
    var g_high_water_resultid = <?php echo high_water_resultid(); ?>;
    var g_high_water_group = <?php echo $high_water_rounds['roundid']; ?>;
    var g_use_master_sched = <?php echo $use_master_sched ? 1 : 0; ?>;
    var g_using_groupid = false;
</script>
<script type="text/javascript" src="update.js"></script>
<title>Results By Racer <?php if (isset($_GET['racerid'])) echo ' for '.$_GET['racerid']; ?></title>
<?php require('stylesheet.inc'); ?>
</head>
<body>
<?php
$banner_title = 'Results By Racer';
require('banner.inc');

$nlanes = get_lane_count();

$now_running = get_running_round();
running_round_header($now_running, /* Use RoundID */ TRUE);

require_once('rounds.inc');
$rounds = all_rounds();

$sql = 'SELECT registrationinfo.RacerID,'
  .' classes.Class, Round, Heat, Lane, FinishTime, ResultID,'
  .' CarNumber, registrationinfo.FirstName, registrationinfo.LastName,'
  .' classes.ClassID, rounds.RoundID'
  .' FROM classes'
  .' INNER JOIN (rounds'
  .' INNER JOIN (roster'
  .' INNER JOIN (registrationinfo'
  .' INNER JOIN racechart'
  .' ON registrationinfo.RacerID = racechart.RacerID)'
  .' ON roster.RacerID = registrationinfo.RacerID)'
  .' ON rounds.RoundID = roster.RoundID)'
  .' ON rounds.ClassID = classes.ClassID'
  .' WHERE rounds.RoundID = racechart.RoundID'
  .(isset($_GET['racerid'])
    ? ' AND racechart.RacerID = '.$_GET['racerid'] : '')
  .' ORDER BY Class, Round, LastName, FirstName, CarNumber, ResultID, Lane';
$rs = odbc_exec($conn, $sql);
if ($rs === FALSE) {
  echo '<h2>Error: '.odbc_errormsg($conn).'</h2>'."\n";
 }
?>
<table class="main_table">
<?php

function byes($n) {
  $result = '';
  while ($n > 0) {
    $result .= '<td>--</td>';
    --$n;
  }
  return $result;
}

function write_rr($racer_label, $racer_cells, $nrows) {
  global $nlanes;
  global $row;  // read and written

  $rrow = '<tr class="d'.($row & 1).'">'
         .'<th rowspan="'.$nrows.'">'.$racer_label.'</th>';
  ++$row;
  for ($r = 0; $r < $nrows; ++$r) {
    if ($r > 0) {
      $rrow .= '</tr><tr class="d'.($row & 1).'">';
      ++$row;
    }
    foreach ($racer_cells as $cols) {
      if (count($cols) <= $r) {
	// This racer didn't race in this lane (in this round)
	$rrow .= '<td/>'; // '<td>row '.$r.' count '.(count($cols)).'</td>'; // TODO: Bye
      } else {
	$rrow .= $cols[$r];
      }
    }
  }
  $rrow .= '</tr>';
  echo $rrow;
}

$valid = odbc_fetch_row($rs);
foreach ($rounds as $round) {
  // Generate header and tbody
  $roundid = $round['RoundID'];
  $classid = $round['ClassID'];
  $groupid = $round['GroupID'];

  $is_current = 0;
  if ($now_running['roundid'] == $roundid and
      $now_running['classid'] == $classid)
    $is_current = 1;

  echo '<tbody id="group_'.$groupid.'" class="group_'.$groupid.'">'."\n";
  echo '<tr><th/><th class="group_spacer wide" colspan="'.$nlanes.'"/></tr>'."\n";
  echo '<tr><th class="pre_group_title"/>'
      .'<th class="group_title wide" colspan="'.$nlanes.'">'
            .$round['Class'].', Round '.$round['Round'].'</th>'
      .'</tr>'."\n";

  echo '<tr>';
  echo '<th>Racer</th>';
  for ($l = 1; $l <= $nlanes; ++$l)
    echo '<th>Lane '.$l.'</th>';
  echo '</tr>'."\n";

  $row = 1;
  $racerid = 0;
  $racer_label = '';
  while ($valid and odbc_result($rs, 'RoundID') == $roundid) {
    if ($racerid <> odbc_result($rs, 'RacerID')) {
      if ($racer_label) {
	write_rr($racer_label, $racer_cells, $nrows);
      }
      $racerid = odbc_result($rs, 'RacerID');
      $racer_label = '<span class="racer">'
	.odbc_result($rs, 'LastName')
	.', '.odbc_result($rs, 'FirstName').'</span>'
	.' (<span class="car">'.odbc_result($rs, 'CarNumber').'</span>)';
      $racer_cells = array();
      for ($i = 1; $i <= $nlanes; ++$i) {
	$racer_cells[] = array();
      }
      //++$row;
      $lane = 1;
      $nrows = 1;
    }

    $lane = odbc_result($rs, 'Lane');

    $ft = odbc_result($rs, 'FinishTime');
    $racer_cells[$lane - 1][] = '<td class="resultid_'.odbc_result($rs, 'ResultID').'">'
                 .'<a class="heat_link" href="ondeck.php#heat_'.$roundid.'_'.odbc_result($rs, 'Heat').'">'
                 .'<span class="time">'.($ft ? number_format($ft, 3) : '--').'</span>'
                 .'</a>'
                 .'</td>'."\n";
    if (count($racer_cells[$lane - 1]) > $nrows) {
      $nrows = count($racer_cells[$lane - 1]);
    }
    ++$lane;
    $valid = odbc_fetch_row($rs);
  }

  if ($racer_label) {
    write_rr($racer_label, $racer_cells, $nrows);
  }
  echo '</tbody>'."\n";
}

odbc_close($conn);
?>
</table>
<div class="block_buttons">
<?php
  if (isset($_GET['racerid'])) {
    echo '<form method="link" action="racer-results.php">'."\n";
    echo '<input type="submit" value="View All Racers"/>'."\n";
    echo '</form>'."\n";
  }
?>
</div>
</body>
</html>