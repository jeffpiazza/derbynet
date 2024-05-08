<?php @session_start();
require_once('inc/banner.inc');
require_once('inc/data.inc');
session_write_close();
?><!DOCTYPE html>
<html>
<head>
<meta http-equiv="Content-Type" content="text/html; charset=UTF-8"/>
<title>Replay Camera</title>
<link rel="stylesheet" type="text/css" href="css/jquery-ui.min.css"/>
<?php require('inc/stylesheet.inc'); ?>
<link rel="stylesheet" type="text/css" href="css/camera.css"/>
<script type="text/javascript" src="js/jquery.js"></script>
<script type="text/javascript" src="js/jquery-ui.min.js"></script>
<script type="text/javascript" src="js/adapter.js"></script>
<script type="text/javascript" src="js/message-poller.js"></script>
<script type="text/javascript" src="js/camera-signaling.js"></script>
<script type="text/javascript" src="js/video-device-picker.js"></script>
<script type="text/javascript">

g_websocket_url = <?php echo json_encode(read_raceinfo('_websocket_url', '')); ?>;

function logmessage(txt) {
  $("<p></p>").text(txt).appendTo($("#log"));
  let msgs = $("#log p");
  if (msgs.length > 20) {
    msgs.slice(0, -20).remove();
  }
}

function on_add_client() {
  // Adding a new client may give new information about the ideal width and
  // height for the video stream.
  on_device_selection($("#device-picker"));
}

var g_client_manager = new ViewClientManager(on_add_client);

function on_device_selection(selectq) {
  // mobile_select_refresh(selectq);
  // If a stream is already open, stop it.
  stream = document.getElementById("preview").srcObject;
  if (stream != null) {
    stream.getTracks().forEach(function(track) {
      track.stop();
    });
  }	
  
  let device_id = selectq.find(':selected').val();
  let video_constraint = {deviceId: device_id};
  let ideal = g_client_manager.ideal();
  if (ideal.width) {
    video_constraint.width = {ideal: ideal.width};
  }
  if (ideal.height) {
    video_constraint.height = {ideal: ideal.height};
  }
  navigator.mediaDevices.getUserMedia({video: video_constraint})
  .then(stream => {
      document.getElementById("preview").srcObject = stream;
      g_client_manager.setstream(stream);

      let tracks = stream.getVideoTracks();
      if (tracks.length > 0) {
        logmessage("Video stream width is " + tracks[0].getSettings().width);
      }
    });
}

$(function() {
    if (typeof(navigator.mediaDevices) == 'undefined') {
      $("#no-camera-warning").removeClass('hidden');
      if (window.location.protocol == 'http:') {
        var https_url = "https://" + window.location.hostname + window.location.pathname;
        $("#no-camera-warning").append("<p>You may need to switch to <a href='" +  https_url + "'>" + https_url + "</a></p>");
      }
    } else {
      navigator.mediaDevices.ondevicechange = function(event) {
        build_device_picker($("#device-picker"), /*include_remote*/false, on_device_selection);
      };
    }

    build_device_picker($("#device-picker"), /*include_remote*/false, on_device_selection);
});

function toggle_preview() {
  $("#preview").toggleClass('hidden', !document.getElementById('show-preview').checked);
  return false;
}

</script>
</head>
<body>
<?php make_banner('Replay Camera'); ?>

<div id="log"></div>

<div class="block_buttons">
  <video id="preview" autoplay="true" muted="true" playsinline="true"></video>

  <div id="no-camera-warning" class="hidden">
     <h2 id='reject'>Access to cameras is blocked.</h2>
  </div>

  <div id="device-picker-div">
    <select id="device-picker"><option>Please wait</option></select>
  </div>

  <div>
    <input type="checkbox" id="show-preview" checked="checked"
        onclick="toggle_preview();"/>
    <label for="show-preview">Show preview?</label>
  </div>
</div>

</body>
</html>
