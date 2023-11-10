<?php
require_once('inc/banner.inc');
require_once('inc/data.inc');
require_once('inc/json-current-racers.inc');
require_once('inc/name-mangler.inc');

$nlanes = get_lane_count();

?><!DOCTYPE html>
<html>
<head>
<meta http-equiv="Content-Type" content="text/html; charset=UTF-8"/>
<title>Derby Racing Main Board</title>
<?php require('inc/stylesheet.inc'); ?>
<link rel="stylesheet" type="text/css" href="css/now-racing-columnar.css"/>
<script type="text/javascript">
<?php echo $lane_to_column_fn; ?>
</script>
<script type="text/javascript" src="js/jquery.js"></script>
<script type="text/javascript" src="js/jquery-ui.min.js"></script>
<?php require('inc/kiosk-poller.inc'); ?>
<script type="text/javascript" src="js/now-racing-overlay.js"></script>
<script type="text/javascript" src="js/now-racing-columnar.js"></script>
<script type="text/javascript">
var g_number_of_lanes = <?php echo $nlanes; ?>;
if (g_number_of_lanes == 0) {
  setTimeout(function() {
      console.log("Reloading because nlanes not set yet.");
      location.reload(true);
    },
    5000);
}

var g_column_width_exact = $(window).width() / (g_number_of_lanes == 0 ? 1 : g_number_of_lanes);
var g_column_width = Math.round(g_column_width_exact);

// 29vh (div.car) - 5vh (div.number, the car number)
var g_car_photo_height = Math.floor(1 + 0.24 * $(window).height());
// 38vh (div.racer) - 4vh (div.name)
var g_racer_photo_height = Math.floor(1 + 0.34 * $(window).height());

$(function() {
  for (lane = 1; lane <= g_number_of_lanes; ++lane) {
    $("div.lane").eq(lane_to_column(lane)).find("div.lane_number").text(lane);
  }
});
</script>
</head>
<body>
<?php make_banner('Racing', false); ?>

<div id="all-lanes">
<?php
$now_running = get_running_round();

for ($lane = 1; $lane <= $nlanes; ++$lane) {
?>

    <div class="lane">
      <div class="lane_number">X</div>
      <div class="rollable">
<?php  // Don't be fooled -- this is just the initial, empty racer-entry for each lane.
       // Real content is created by javascript code dynamically. ?> 

        <div class="racer-entry" data-heat-key="" data-racerid="">
          <div class="heat_time"> &nbsp; </div>
          <div class="car">
            <div class="name"></div>
          </div>
          <div class="racer">
            <div class="name"></div>
          </div>
        </div>

      </div>
    </div>

<?php
}
?>
</div>

<div class="bottom-banner">
    <div class="label">Best heat times</div>
</div>

<div id="overlay_background"></div>
<div id="paused_overlay" class="overlay_foreground">
 <img src="img/pause.png"/>
</div>
<div id="timer_overlay" class="overlay_foreground">
 <img src="img/timer-red.png"/>
 <p>Check timer.</p>
</div>
<div id="reschedule_overlay" class="overlay_foreground">
 <img src="img/warning-icon.png"/>
 <p>Schedule adjustment needed.</p>
</div>

<?php require_once('inc/ajax-failure.inc'); ?>

</body>
</html>
