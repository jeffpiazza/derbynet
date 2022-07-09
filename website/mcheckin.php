<?php @session_start();
require_once('inc/banner.inc');
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
<?php make_banner('Mobile Check-In'); ?>

<video id="preview" autoplay muted playsinline>
</video>

<audio id="beep" src="img/barcode.mp3"></audio>


<div id="device-picker-div">
  <select id="device-picker" class="not-mobile"><option>Please wait</option></select>
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
