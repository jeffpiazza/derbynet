<?php @session_start();
require_once('inc/banner.inc');
require_once('inc/authorize.inc');

$_SESSION['permissions'] |= CHECK_IN_RACERS_PERMISSION | REVERT_CHECK_IN_PERMISSION | PHOTO_UPLOAD_PERMISSION;

?><!DOCTYPE html>
<html>
<head>
<meta http-equiv="Content-Type" content="text/html; charset=UTF-8"/>
<title>Mobile Check-In</title>
<link rel="stylesheet" type="text/css" href="css/jquery-ui.min.css"/>
<?php require('inc/stylesheet.inc'); ?>
<link rel="stylesheet" type="text/css" href="css/mobile.css"/>
<link rel="stylesheet" type="text/css" href="css/camera.css"/>
<link rel="stylesheet" type="text/css" href="css/mcheckin.css"/>
<script type="text/javascript" src="js/jquery.js"></script>
<script type="text/javascript" src="js/jquery-ui.min.js"></script>
<script type="text/javascript" src="js/mobile.js"></script>
<!-- webrtc adapter: recommended for use with quaggajs -->
<script type="text/javascript" src="js/adapter.js"></script>
<script type="text/javascript" src="js/video-device-picker.js"></script>
<script type="text/javascript" src="js/zxing.js"></script>
<script type="text/javascript" src="js/mcheckin.js"></script>
</head>
<body>
<?php make_banner('Mobile Check-In', 'checkin.php'); ?>

<div id="instructions-background">
  <div id="instructions" class="block_buttons">
    <p id="step1">Point your camera at the barcode identifying a racer.  Then:</p>
    <p>Tap <img src="img/photo-headshot.png"/> to capture a picture of the racer.</p>
    <p>Use the switch to mark the racer as checked in.</p>
    <p>Tap <img src="img/photo-car.png"/> to capture a picture of the car.</p>

    <input type="button" value="Start" onclick="start_camera()"/>
 </div>
</div>


<div id="trouble" class="hidden" onclick="close_trouble();">
</div>

<video id="preview" autoplay muted playsinline>
</video>

<div id="instructions-overlay" class="hidden">
Orient barcode like this.
</div>
<div id="instructions-overlay-background" class="hidden">
</div>


<audio id="beep" src="img/barcode.mp3"></audio>


<div id="device-picker-div">
  <select id="device-picker" ><option>Please wait</option></select>
</div>

<div id="slide-up">
  <div id="racer-div"></div>
  <div id="controls">
     <div id="controls-inner">
      <div id="head-button" class="escutcheon" onclick="capture_photo('head');" style="margin-left: 0px;">
        <img src="img/photo-headshot.png"/>
      </div>
      <div class="escutcheon" onclick="on_checkin_button();">
        <div class="click-blocker"> &nbsp; </div>
        <input id="checked_in" type="checkbox" class="flipswitch"/>
      </div>
      <div id="car-button" class="escutcheon" onclick="capture_photo('car');">
        <img src="img/photo-car.png"/>
      </div>
    </div>
  </div>
</div>

<div id="flash-overlay"></div>

<canvas id="canvas"></canvas>

</body>
</html>
