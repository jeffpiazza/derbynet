<?php @session_start(); ?>
<?php
require_once('inc/data.inc');
require_once('inc/authorize.inc');
session_write_close();
require_once('inc/banner.inc');
require_once('inc/plural.inc');

require_permission(CHECK_IN_RACERS_PERMISSION);

?><!DOCTYPE html>
<html>
<head>
<meta http-equiv="Content-Type" content="text/html; charset=UTF-8"/>
<title>Fake Timer</title>
<?php require('inc/stylesheet.inc'); ?>
<link rel="stylesheet" type="text/css" href="css/mobile.css"/>
<script type="text/javascript" src="js/jquery.js"></script>
<script type="text/javascript" src="js/mobile.js"></script>
<script type="text/javascript" src="js/fake-timer.js"></script>
<style type="text/css">
p.frontmatter {
  margin-left: auto;
  margin-right: auto;
  width: 500px;
}

#lane-count-div {
  width: 300px;
  margin-left: auto;
  margin-right: auto;
}
#lane-count-div label {
  font-size: 20px;
  line-height: 1.3;
}
#lane-count {
  display: block;
  margin-left: auto;
  margin-right: auto;
  font-size: 24px;
  line-height: 1.3;
  /* Weirdly, this causes the control to widen considerably in Safari,
     much more than the text would require. */
  font-family: sans-serif;
}

p#summary {
  font-size: 36px;
  text-align: center;
  width: 500px;
  margin-left: auto;
  margin-right: auto;
}

p#summary .heatno,
p#summary .round-class,
p#summary .roundno {
  font-weight: bold;
  font-size: inherit;
}

#timer-sim {
  margin-left: auto;
  margin-right: auto;
}

#timer-sim-times {
  font-size: 48px;
}

#timer-sim input[type='button'] {
  width: 150px;
}

#auto-mode-outer-div {
  margin-top: 50px;
  margin-left: auto;
  margin-right: auto;
  width: 500px;
  font-size: normal;
}
#auto-mode-div {
  width: 300px;
  margin-left: auto;
  margin-right: auto;
}
</style>
</head>
<body>
<?php make_banner('Fake Timer', false /* no back button */); ?>

<p class='frontmatter'>This page simulates the behavior of a timer connected to the DerbyNet server.
It's intended to allow you to get a feel for using DerbyNet without having to connect
a real timer.</p>

<p class='frontmatter'>The fake timer operates <b>only while this page is open</b>,
and <b>only when
<a href="coordinator.php" target="_blank">"Racing"</a> or
<a href="timer.php" target="_blank">"Simulated Racing"</a> is underway.</b></p>

<p class='frontmatter'>We <b><i>strongly</i></b> encourage connecting and testing your real timer
before the day of your actual race.</p>

<div id='lane-count-div'>
  <label for="lane-count">Number of lanes on the track:</label>
  <input id="lane-count" type="number" class="not-mobile" min="2" max="8"
         value="<?php echo get_lane_count(); ?>"/>
</div>

<div class='block_buttons'>

<p id="summary">Not racing.</p>

<input id='start-button' type='button' value='Start' onclick='start_timer();' disabled='1'/>

<table id='timer-sim'>
  <tr id='timer-sim-headers'>
      <th>Lane 1</th>
      <th>Lane 2</th>
      <th>Lane 3</th>
      <th>Lane 4</th>
  </tr>

  <tr id='timer-sim-times' data-time='0'>
      <td data-running='1'/>
      <td data-running='1'/>
      <td data-running='1'/>
      <td data-running='1'/>
  </tr>

  <tr id='timer-sim-stop-buttons'>
      <td><input type='button' value='Stop' onclick='stop_timer(this);'/></td>
      <td><input type='button' value='Stop' onclick='stop_timer(this);'/></td>
      <td><input type='button' value='Stop' onclick='stop_timer(this);'/></td>
      <td><input type='button' value='Stop' onclick='stop_timer(this);'/></td>
  </tr>
</table>

</div>

<div id='auto-mode-outer-div'>

  <p>"Auto mode" simulates a very efficient race crew that stages and starts
     each heat within a few seconds after it's set ready from the server.</p>

  <div id='auto-mode-div'>
      <label for='auto-mode-checkbox'>Auto Mode: </label>
      <input id='auto-mode-checkbox' type='checkbox' class='flipswitch'/>
  </div>
</div>

</body>
</html>
