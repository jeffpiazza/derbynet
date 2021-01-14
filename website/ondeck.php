<?php
// TODO - Write a photo src template as a data attribute instead of as the src,
//        then convert to a square photo size based on nlanes
//
// When presented as a kiosk page, i.e., when this php file is included from
// kiosks/ondeck.kiosk, the session_start() function will already have been
// called.  The @ is necessary to suppress the error notice that may arise in
// this case.
@session_start();
require_once('inc/data.inc');
require_once('inc/banner.inc');
require_once('inc/authorize.inc');
require_once('inc/photo-config.inc');
require_once('inc/name-mangler.inc');
require_once('inc/schema_version.inc');
require_once('inc/running_round_header.inc');

    $nlanes = get_lane_count_from_results();
    $now_running = get_running_round();
    $use_master_sched = use_master_sched();

    $repo = car_photo_repository();

    $high_water_rounds = high_water_rounds();

    $show_car_photos = read_raceinfo_boolean('show-cars-on-deck');
?><!DOCTYPE html>
<html>
<head>
<meta http-equiv="Content-Type" content="text/html; charset=UTF-8"/>
<script type="text/javascript" src="js/jquery.js"></script>
<script type="text/javascript" src="js/ajax-setup.js"></script>
<script type="text/javascript" src="js/modal.js"></script>
<?php if (isset($as_kiosk)) {
  require_once('inc/kiosk-poller.inc');
  echo "<style type='text/css'>\n";
  echo "body { overflow: hidden; }\n";
  echo "</style>\n";
}?>
<script type="text/javascript">
var g_update_status = {
      last_update_time: "", // First refresh is for everything
      high_water_resultid: <?php echo high_water_resultid(); ?>,
      high_water_tbodyid: <?php
        echo $high_water_rounds[$use_master_sched ? 'round' : 'roundid']; ?>,
      use_master_sched: <?php echo $use_master_sched ? 1 : 0; ?>,
      merge_rounds: <?php echo $use_master_sched ? 1 : 0; ?>,
};
// g_focus should be one of 'current', 'next', 'previous', or ''.  The default
// is g_focus = '', which acts like 'next' but only if that row is visible.
// (Allows user to scroll to a different row and not have to fight the
// autoscrolling.)
var g_focus = <?php
    if (isset($_GET['focus'])) {
      echo json_encode(empty($_GET['focus']) ? 'next' : $_GET['focus']);
    } else if (isset($_GET['focus_current'])) {
      // Legacy
      echo '"current"';
    } else {
      echo '""';
    }
?>;
</script>
<script type="text/javascript" src="js/common-update.js"></script>
<script type="text/javascript" src="js/ondeck-update.js"></script>
<script type="text/javascript">

function handle_row_click(tr) {
  $(tr).toggleClass('exposed');
}

function handle_photo_click(img) {
  $("#photo_view_img").attr('src', $(img).attr('data-img'));
  show_modal("#photo_view_modal", function() {});
}

</script>
<title>Race Schedule</title>
<?php require('inc/stylesheet.inc'); ?>
<link rel="stylesheet" type="text/css" href="css/kiosks.css"/>
<link rel="stylesheet" type="text/css" href="css/main-table.css"/>
<link rel="stylesheet" type="text/css" href="css/ondeck.css"/>
</head>
<body>
<?php
make_banner('Racing Heats', isset($as_kiosk) ? '' : 'index.php');
running_round_header($now_running);

require_once('inc/rounds.inc');
$groups = all_schedule_groups();

$sql = 'SELECT'
    .' Classes.class, round, heat, lane, finishtime, resultid, completed,'
    .' RegistrationInfo.racerid,'
    .(schema_version() < 2 ? ' \'\' as' : '').' carphoto, '
    .($use_master_sched ? 'round' : 'Rounds.roundid').' as racinggroup,'
    .($use_master_sched ? 'masterheat' : 'heat').' as seq,'
    .' RegistrationInfo.carnumber, RegistrationInfo.firstname, RegistrationInfo.lastname,'
    .' Classes.classid, Rounds.roundid, RaceChart.racerid'
    .' FROM '.inner_join('RaceChart', 'RegistrationInfo', 
                         'RegistrationInfo.racerid = RaceChart.racerid',
                         'Roster', 'Roster.racerid = RegistrationInfo.Racerid',
                         'Rounds', 'Rounds.roundid = Roster.roundid',
                         'Classes', 'Rounds.classid = Classes.classid')
    .' WHERE Rounds.roundid = RaceChart.roundid'
    .' ORDER BY '
    .($use_master_sched
      ? 'round, masterheat, lane'
      : ((schema_version() >= 2 ? 'Classes.sortorder, ' : '')
         .'class, round, heat, lane'));

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
    $result .= '<td>Bye<div class="ondeck_photo unpopulated"></div></td>';
    --$n;
  }
  return $result;
}

// The $row_counter gets incremented for each row shown; its parity determines
// the row color.  seq field in RaceChart orders the heats, but may have gaps
// due to deleted schedules, so can't use that.
$row_counter = 0;

function write_heat_row($entry, $heat_row, $lane) {
  global $row_counter;
  global $nlanes;
  global $use_master_sched;

  if ($entry) {
    $heat_row .= byes($nlanes - $lane + 1);
    $heat = $entry['heat'];
    $heat_label = 'heat_'.$entry['roundid'].'_'.$heat;
    echo '<tr id="'.$heat_label.'" class="heat_row d'.($row_counter & 1).'"'
            .' onclick="handle_row_click(this);">'
      .'<th>'
      .htmlspecialchars(($use_master_sched ? $entry['class'].' ' : '')
                        .'Heat '.$heat, ENT_QUOTES, 'UTF-8')
      .'</th>'
      .$heat_row.'</tr>'."\n";
    ++$row_counter;
  }
}

function maybe_mark_photos_populated($heat_row, $photo_in_heat) {
  if ($photo_in_heat) {
    $heat_row = str_replace('ondeck_photo unpopulated', 'ondeck_photo populated', $heat_row);
  }
  return $heat_row;
}

$name_style = read_raceinfo('name-style', FULL_NAME);

$rs = $stmt->fetch(PDO::FETCH_ASSOC);
foreach ($groups as $group) {
  // Generate header and tbody
  $roundno = $group['round'];
  $groupid = $group['groupid'];

  echo '<tbody id="tbody_'.$groupid.'">'."\n";
  echo '<tr><th/><th class="group_spacer wide" colspan="'.$nlanes.'"/></tr>'."\n";
  echo '<tr><th class="pre_group_title"/>'
      .'<th class="group_title wide" colspan="'.$nlanes.'">'
      .htmlspecialchars($group['roundname'], ENT_QUOTES, 'UTF-8').'</th>'
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
  // $photos_in_heat turns true if any entrant in the current heat has a car photo populated
  $photos_in_heat = false;
  while ($rs and $rs['racinggroup'] == $groupid) {
    if ($seq <> $rs['seq']) {
      write_heat_row($first_entry, maybe_mark_photos_populated($heat_row, $photos_in_heat), @$lane);
      $heat_row = '';
      $seq = $rs['seq'];
      $first_entry = array('roundid' => $rs['roundid'],
						   'heat' => $rs['heat'],
						   'class' => $rs['class'],
						   'seq' => $seq);
      $lane = 1;
      $photos_in_heat = false;
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
      $heat_row .= '<td class="lane_'.$lane.' resultid_'.$rs['resultid'].'">';
      // $heat_row .= '<a class="racer_link" href="racer-results.php?racerid='.$rs['racerid'].'">';
      $heat_row .= '<span class="car">'.htmlspecialchars($rs['carnumber'], ENT_QUOTES, 'UTF-8').'</span><br/>'."\n";
      $heat_row .= '<span class="racer">('
          .htmlspecialchars(mangled_name($rs, $name_style), ENT_QUOTES, 'UTF-8').')</span><br/>'."\n";
      $heat_row .= '<span class="time"></span>'; // Javascript will fill in the times, later
      // $heat_row .= '</a>';
      $heat_row .= '<div class="ondeck_photo unpopulated">';
      if ($show_car_photos && isset($rs['carphoto']) && $rs['carphoto']) {
        $photos_in_heat = true;
        $heat_row .= '<img src="'.$repo->url_for_racer($rs, RENDER_ONDECK).'"'
                    .' data-img="'.$repo->url_for_racer($rs, RENDER_WORKING).'"'
                    .' onclick="handle_photo_click(this)"/>';
      }
      $heat_row .= '</div>';

      $heat_row .= '</td>';
      ++$lane;
    } else {
      echo '<tr>'
	.'<th class="unsched wide" colspan="'.$nlanes.'">Heats have not yet been scheduled.</th>'
	.'</tr>'."\n";
    }
    $rs = $stmt->fetch(PDO::FETCH_ASSOC);
  }

  write_heat_row($first_entry, maybe_mark_photos_populated($heat_row, $photos_in_heat), @$lane);

  echo '</tbody>'."\n";
}

$stmt->closeCursor();
?>
</table>

<?php require_once('inc/ajax-failure.inc'); ?>

<div id='photo_view_modal' class="modal_dialog hidden block_buttons">
  <img id='photo_view_img'/>
  <br/>
  <input type="button" value="Close"
    onclick='close_modal("#photo_view_modal");'/>
</div>

</body>
</html>