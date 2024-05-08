<?php @session_start();
require_once('inc/data.inc');
session_write_close();
require_once('inc/banner.inc');
require_once('inc/photo-config.inc');
require_once('inc/name-mangler.inc');
require_once('inc/schema_version.inc');
require_once('inc/running_round_header.inc');
require_once('inc/ordinals.inc');
require_once('inc/rounds.inc');

$use_master_sched = use_master_sched();
$high_water_rounds = high_water_rounds();

$signatures = schedule_signature();

$limit_to_roundid = false;
if (isset($as_kiosk)) {
  $params = kiosk_parameters();
  if (isset($params['roundid'])) {
    $limit_to_roundid = $params['roundid'];
  }
}
?><!DOCTYPE html>
<html>
<head>
<meta http-equiv="Content-Type" content="text/html; charset=UTF-8"/>
<script type="text/javascript" src="js/jquery.js"></script>
<script type="text/javascript" src="js/ajax-setup.js"></script>
<script type="text/javascript">

var g_as_kiosk = <?php echo isset($as_kiosk) ? "true" : "false"; ?>;

var g_limited_to_roundid = <?php echo $limit_to_roundid ? "true" : "false"; ?>;

</script>
<?php
if (isset($as_kiosk)) {
  require_once('inc/kiosk-poller.inc');
  echo "<style type='text/css'>\n";
  echo "body { overflow: hidden; }\n";
  echo "</style>\n";
}
?>
<script type="text/javascript" src="js/common-update.js"></script>
<script type="text/javascript" src="js/results-by-racer-update.js"></script>
<?php if (isset($as_kiosk))
    echo '<script type="text/javascript" src="js/results-by-racer-scrolling.js"></script>'."\n";
?>
<title>Results By Racer <?php
    if (isset($_GET['racerid']) && is_numeric($_GET['racerid'])) {
      echo ' for '.$_GET['racerid'];
    } ?></title>
<?php require('inc/stylesheet.inc'); ?>
<link rel="stylesheet" type="text/css" href="css/kiosks.css"/>
<link rel="stylesheet" type="text/css" href="css/main-table.css"/>
<link rel="stylesheet" type="text/css" href="css/racer-results.css"/>
</head>
<body>
<?php
make_banner('Results By Racer', isset($as_kiosk) ? '' : 'index.php');

$nlanes = get_lane_count_from_results();

$now_running = get_running_round();
if (!$limit_to_roundid) {
    running_round_header($now_running, /* Use RoundID */ true);
}

$show_racer_photos = read_raceinfo_boolean('show-racer-photos-rr');
$show_car_photos = read_raceinfo_boolean('show-car-photos-rr');
$use_points = read_raceinfo_boolean('use-points');

$rounds = all_rounds();

$sql = 'SELECT RegistrationInfo.racerid,'
    .' Classes.class, round, heat, lane, finishtime, finishplace, resultid,'
    .' carnumber, firstname, lastname, imagefile, '
    .(schema_version() >= 2 ? 'carphoto, ' : '')
    .' Classes.classid, Rounds.roundid'
    .' FROM '.inner_join('RaceChart', 'RegistrationInfo',
                         'RegistrationInfo.racerid = RaceChart.racerid',
                         'Roster', 'Roster.racerid = RegistrationInfo.racerid',
                         'Rounds', 'Rounds.roundid = Roster.roundid',
                         'Classes', 'Rounds.classid = Classes.classid')
    .' WHERE Rounds.roundid = RaceChart.roundid'
    .($limit_to_roundid !== false ? " AND Rounds.roundid = $limit_to_roundid" : '')
    .(isset($_GET['racerid']) ? " AND RaceChart.racerid = $_GET[racerid]" : '')
    .' ORDER BY '
    .(schema_version() >= 2 ? 'Classes.sortorder, ' : '')
    .'class, round, lastname, firstname, carnumber, resultid, lane';

$stmt = $db->query($sql);
if ($stmt === FALSE) {
  $info = $db->errorInfo();
  echo '<h2>Error: '.$info[2].'</h2>'."\n";
 }
?>
<div class="scroll-bounding-rect">
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

function write_rr($racer_label, $roundid, $racer_cells, $nrows) {
  global $nlanes;
  global $row;  // read and written

  $rrow = '<tr class="d'.($row & 1).'" data-roundid="'.$roundid.'">'
         .'<th rowspan="'.$nrows.'" class="nrows'.$nrows.'">'.$racer_label.'</th>';
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

$name_style = read_name_style();
$time_format = get_finishtime_formatting_string();

$rs = $stmt->fetch(PDO::FETCH_ASSOC);
foreach ($rounds as $round) {
  // Generate header and tbody
  $roundid = $round['roundid'];
  $classid = $round['classid'];
  $groupid = $round['groupid'];

  $is_current = 0;
  if ($now_running['roundid'] == $roundid and
      $now_running['classid'] == $classid)
    $is_current = 1;

  // For this page, 'groupid' is always roundid, regardless of master scheduling or not.
  echo '<tbody id="tbody_'.$groupid.'" data-roundid="'.$groupid.'" data-signature="'.$signatures[$groupid].'">'."\n";
  echo '<tr><th/><th class="group_spacer wide" colspan="'.$nlanes.'"/></tr>'."\n";
  echo '<tr><th class="pre_group_title"/>'
      .'<th class="group_title wide" colspan="'.$nlanes.'">'
      .(use_groups()
        ? htmlspecialchars($round['class'], ENT_QUOTES, 'UTF-8').', '
        : '')
      .'Round '.$round['round'].'</th>'
      .'</tr>'."\n";

  echo '<tr>';
  echo '<th>Racer</th>';
  for ($l = 1; $l <= $nlanes; ++$l)
    echo '<th>Lane '.$l.'</th>';
  echo '</tr>'."\n";

  $row = 1;
  $racerid = 0;
  $racer_label = '';
  while ($rs and $rs['roundid'] == $roundid) {
    if ($racerid <> $rs['racerid']) {
      if ($racer_label) {
		write_rr($racer_label, $roundid, $racer_cells, $nrows);
      }
      $racerid = $rs['racerid'];
      $racer_label = '';
      // 68h images completely fill one row's height
      if ($rs['imagefile'] && $show_racer_photos) {
        $head_url = headshots()->url_for_racer($rs, RENDER_RACER_RESULTS);
        $racer_label .= "<img src=\"$head_url\" class=\"racer-photo\"/>";
      }
      if (isset($rs['carphoto']) && $rs['carphoto'] && $show_car_photos) {
        $car_url = car_photo_repository()->url_for_racer($rs, RENDER_RACER_RESULTS);
        $racer_label .= "<img src=\"$car_url\" class=\"racer-photo\"/>";
      }
      $racer_label .= '<div class="racer_label"><span class="racer">'
        .htmlspecialchars(mangled_name($rs, $name_style), ENT_QUOTES, 'UTF-8').'</span>'
		.' (<span class="car">'.$rs['carnumber'].'</span>)</div>';
      $racer_cells = array();
      for ($i = 1; $i <= $nlanes; ++$i) {
		$racer_cells[] = array();
      }
      //++$row;
      $lane = 1;
      $nrows = 1;
    }

    $lane = $rs['lane'];

    $ft = $use_points ? (isset($rs['finishplace']) ? ordinal($rs['finishplace']) : '--')
                      : (isset($rs['finishtime']) ? sprintf($time_format, $rs['finishtime']) : '--');

    $racer_cells[$lane - 1][] = '<td class="resultid_'.$rs['resultid'].'">'
    //.'<a class="heat_link" href="ondeck.php#heat_'.$roundid.'_'.$rs['heat'].'">'
                 .'<span class="time">'.$ft.'</span>'
    //.'</a>'
                 .'</td>'."\n";
    if (count($racer_cells[$lane - 1]) > $nrows) {
      $nrows = count($racer_cells[$lane - 1]);
    }
    ++$lane;
    $rs = $stmt->fetch(PDO::FETCH_ASSOC);
  }

  if ($racer_label) {
    write_rr($racer_label, $roundid, $racer_cells, $nrows);
  }
  echo '</tbody>'."\n";
}

$stmt->closeCursor();
?>
</table>
</div>
<div class="block_buttons">
<?php
  if (isset($_GET['racerid'])) {
    echo '<a class="button_link" href="racer-results.php">View All Racers</a>'."\n";
  }
?>
</div>
<?php require_once('inc/ajax-failure.inc'); ?>
</body>
</html>
