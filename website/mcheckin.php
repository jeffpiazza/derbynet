<?php @session_start();
require_once('inc/banner.inc');
require_once('inc/authorize.inc');

$_SESSION['permissions'] |=
    CHECK_IN_RACERS_PERMISSION |
    REVERT_CHECK_IN_PERMISSION |
    PHOTO_UPLOAD_PERMISSION
;
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
<script type="text/javascript">
//window.addEventListener('resize', function(event) { report_trouble('resize event'); });
//window.addEventListener('orientationchange', function(event) { report_trouble('orientationchange event'); });
$(function() {
  refresh_racer_list();
});
</script>
</head>
<body>
<?php make_banner('Mobile Check-In', 'checkin.php'); ?>

<div id="greetings-background" class="hidden"><!-- Added "hidden" as an experiment -->
  <div id="greetings" class="block_buttons">
    <p id="step1">Point your camera at the barcode identifying a racer.  Then:</p>
    <p>Tap <img src="img/photo-headshot.png"/> to capture a picture of the racer.</p>
    <p>Use the switch to mark the racer as checked in.</p>
    <p>Tap <img src="img/photo-car.png"/> to capture a picture of the car.</p>

    <input type="button" value="Start" onclick="dismiss_greetings()"/>
 </div>
</div>


<div id="trouble" class="hidden" onclick="close_trouble();"> </div>

<div id="announce-hover" class="hidden" onclick="close_announce_hover();"></div>

<div id="camera-div" class='hidden'>
  <div id="device-picker-div">
    <select id="device-picker" ><option>Please wait</option></select>
  </div>

  <div id="preview-container">
    <video id="preview" autoplay muted playsinline onclick="on_preview_click()"> </video>
    <div id="preview-overlay">
      <div id="preview-overlay-content">
         Orient barcode like this.
      </div>
    </div>
  </div>
</div>

<div id="racer-list">
  <ul class="mlistview"></ul>
  <div id="switch-to-camera-button" onclick="on_switch_to_camera_cllicked();">Switch to Camera</div>
</div>

<audio id="beep" src="img/barcode.mp3"></audio>
<div id="flash-overlay"></div>

<div id="slide-in">
  <div id="slide-in-inner">
    <div id="racer-div"></div>
    <div id="controls">
      <div id="controls-inner">

        <div id="head-button" class="escutcheon" onclick="capture_photo('head');">
          <div class="thumbnail-carrier" >
            <img class="thumbnail" src="img/photo-headshot.png"/>
          </div>
        </div>
        <div id="autocrop-button" class="escutcheon passed" onclick="on_autocrop_button_click();">
          <div id="autocrop-holder">
            <img src="img/autocrop.png"/>
            <div id="autocrop-overlay"></div>
          </div>
        </div>

        <div id="checkin-button" class="escutcheon" onclick="on_checkin_button_click();">
          <div id="checkin-button-text">No racer selected</div>
        </div>

        <div id="car-button" class="escutcheon" onclick="capture_photo('car');">
          <div class="thumbnail-carrier">
            <img class="thumbnail" src="img/photo-car.png"/>
          </div>
        </div>
      </div>
    </div>
  </div>
</div>


<img id="logo-bug" src="img/derbynet.png"/>

</body>
</html>
