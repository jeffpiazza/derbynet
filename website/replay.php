<?php @session_start();
require_once('inc/banner.inc');
?><!DOCTYPE html>
<html>
<head>
<meta http-equiv="Content-Type" content="text/html; charset=UTF-8"/>
<title>Replay Recording</title>
<?php require('inc/stylesheet.inc'); ?>
<link rel="stylesheet" type="text/css" href="css/replay.css"/>
<script type="text/javascript" src="js/jquery.js"></script>
<script type="text/javascript" src="js/screenfull.min.js"></script>
<script type="text/javascript" src="https://webrtc.github.io/adapter/adapter-latest.js"></script>
<script type="text/javascript" src="js/message-poller.js"></script>
<script type="text/javascript" src="js/viewer-signaling.js"></script>
<script type="text/javascript" src="js/video-capture.js"></script>
<script type="text/javascript" src="js/video-device-picker.js"></script>
<script type="text/javascript">

// TODO Poll for messages.
// In the meantime, use the existing replay protocol
function poll_as_replay() {
  $.ajax("action.php",
  {type: 'POST',
    data: {action: 'replay-message',
           status: 0,
           'finished-replay': 0},
    success: function(data) {
      let msgs = data.getElementsByTagName('replay-message');
      for (let i = 0; i < msgs.length; ++i) {
        handle_replay_message(msgs[i].textContent);
      }
    }
  });
}

function handle_replay_message(cmdline) {
  console.log("handle_replay_message: " + cmdline);
  if (cmdline.startsWith("HELLO")) {
  } else if (cmdline.startsWith("TEST")) {
    on_replay();
  } else if (cmdline.startsWith("START")) {
  } else if (cmdline.startsWith("REPLAY")) {
    on_replay();
  } else if (cmdline.startsWith("CANCEL")) {
  } else {
    console.log("Unrecognized replay message: " + cmdline);
  }
}

$(function() { setInterval(poll_as_replay, 250); });

var g_remote_poller;
var g_recorder;

function select_stream(stream) {
  g_recorder = new VideoCaptureFlight(stream, 4, 1000);
  document.getElementById("preview").srcObject = stream;
}

$(function() {
    if (window.MediaRecorder == undefined) {
      $("#replay-setup").empty()
      .append("<h3 id='reject'>This browser does not support MediaRecorder.<br/>" +
              "Please use another browser for replay.</h3>")
      .append("<p>" + navigator.userAgent + "</p>");
      return;
    }

    navigator.mediaDevices.ondevicechange = function(event) {
      // TODO Reconstruct the picker.  Check whether the currently-selected
      // camera is still on the list.
      console.log("ondevicechange");
    };
    
    if (false) {
      // TODO Remote camera not working yet
      let id = make_viewer_id();
      $("#viewerid").text(id);
      g_remote_poller = new RemoteCamera(
        id, function(stream) {
          document.querySelector('#preview').srcObject = stream;
          g_recorder = new VideoCaptureFlight(stream, 4, 1000);
        });
    } else {
      $("#device-picker-div").append(video_device_picker(select_stream));
    }

    $("#playback").on('ended', function(event) {
        $("#interior").removeClass('hidden');
        $("#playback").addClass('hidden');
      });
});

function on_replay() {
  $("#replay-setup").addClass('hidden');  // Should already be hidden

  g_recorder.stop(function(blob) {
      if (blob) {
        document.querySelector("#playback").src = URL.createObjectURL(blob);
        $("#interior").addClass('hidden');
        $("#playback").removeClass('hidden');
      } else {
        console.log("No blob!");
      }
    });
}

function on_proceed() {
  $("#interior").attr('src', $("#inner_url").val());

  if (document.getElementById('go-fullscreen').checked) {
    if (screenfull.enabled) {
      screenfull.request();
    }
  }

  $("#replay-setup").addClass('hidden');
  $("#playback").addClass('hidden');
  $("#interior").removeClass('hidden');
}

</script>
</head>
<body>

<iframe id="interior" class="hidden full-window">
</iframe>

<video id="playback" class="hidden full-window" autoplay muted playsinline>
</video>

<div id="replay-setup" class="full-window block_buttons">
  <?php make_banner('Replay'); ?>
  <video id="preview" autoplay muted playsinline>
  </video>

  <div id="device-picker-div"></div>

  <input type="checkbox" id="go-fullscreen" checked="checked"/>
  <label for="go-fullscreen">Change to fullscreen?</label>
  <br/>
<?php
if (isset($_SERVER['SERVER_NAME'])) {
  $server = $_SERVER['SERVER_NAME'];
} else if (isset($_SERVER['SERVER_ADDR'])) {
  $server = $_SERVER['SERVER_ADDR'];
} else if (isset($_SERVER['LOCAL_ADDR'])) {
  $server = $_SERVER['LOCAL_ADDR'];
} else if (isset($_SERVER['HTTP_HOST'])) {
  $server = $_SERVER['HTTP_HOST'];
} else {
  $addrs = gethostbynamel(gethostname());
  if (count($addrs) > 0) {
    $server = $addrs[0];
  } else {
    $server = ' unknown server name! ';
  }
}

if (isset($_SERVER['REQUEST_URI'])) {
  $path = $_SERVER['REQUEST_URI'];
} else if (isset($_SERVER['PHP_SELF'])) {
  $path = $_SERVER['PHP_SELF'];
} else {
  $path = $_SERVER['SCRIPT_NAME'];
}

$url = $server . $path;

$last = strrpos($url, '/');
if ($last === false) {
  $last = -1;
}

// Don't force http !
$kiosk_url = '//'.substr($url, 0, $last + 1).'kiosk.php';
?>

  <label for="inner_url">URL:</label>
  <input type="text" name="inner_url" id="inner_url" size="100"
         value="<?php echo htmlspecialchars($kiosk_url, ENT_QUOTES, 'UTF-8'); ?>"/>
  <input type="button" data-enhanced="true" value="Proceed"
         onclick="on_proceed();"/>
</div>

</body>
</html>
