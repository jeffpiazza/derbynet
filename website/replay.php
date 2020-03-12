<?php @session_start();
require_once('inc/banner.inc');
require_once('inc/data.inc');
?><!DOCTYPE html>
<html>
<head>
<meta http-equiv="Content-Type" content="text/html; charset=UTF-8"/>
<title>Replay Recording</title>
<?php
require('inc/stylesheet.inc');
require('inc/servername.inc');

$server = server_name();
$path = request_path();

$url = $server . $path;

$last = strrpos($url, '/');
if ($last === false) {
  $last = -1;
}

// Don't force http !
$kiosk_url = '//'.substr($url, 0, $last + 1).'kiosk.php';
if (isset($_REQUEST['address'])) {
  $kiosk_url .= "?address=".$_REQUEST['address'];
}
?>
<link rel="stylesheet" type="text/css" href="css/replay.css"/>
<script type="text/javascript" src="js/jquery.js"></script>
<script type="text/javascript" src="js/ajax-setup.js"></script>
<script type="text/javascript" src="js/jquery-ui-1.10.4.min.js"></script>
<script type="text/javascript" src="js/screenfull.min.js"></script>
<script type="text/javascript" src="js/adapter.js"></script>
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

g_upload_videos = <?php echo read_raceinfo_boolean('upload-videos') ? "true" : "false"; ?>;
g_video_name_root = "";
// If a replay is triggered by timing out after a RACE_STARTS, then ignore any
// subsequent REPLAY messages until the next START.
g_preempted = false;

var g_remote_poller;
var g_recorder;

var g_replay_count;
var g_replay_rate;

var g_replay_length = 4;

// If non-zero, holds the timeout ID of a pending timeout that will trigger a
// replay based on the start of a heat.
var g_replay_timeout = 0;
// Milliseconds short of g_replay_length to start a replay after a race start.
var g_replay_timeout_epsilon = 0;

function handle_replay_message(cmdline) {
  if (cmdline.startsWith("HELLO")) {
  } else if (cmdline.startsWith("TEST")) {
    g_replay_count = parseInt(cmdline.split(" ")[2]);
    g_replay_rate = parseFloat(cmdline.split(" ")[3]);
    on_replay();
  } else if (cmdline.startsWith("START")) {  // Setting up for a new heat
    g_video_name_root = cmdline.substr(6).trim();
    g_preempted = false;
  } else if (cmdline.startsWith("REPLAY")) {
    // REPLAY skipback showings rate
    //  skipback and rate are ignored, but showings we can honor
    // (Must be exactly one space between fields:)
    g_replay_count = parseInt(cmdline.split(" ")[2]);
    g_replay_rate = parseFloat(cmdline.split(" ")[3]);
    if (!g_preempted) {
      on_replay();
    }
  } else if (cmdline.startsWith("CANCEL")) {
  } else if (cmdline.startsWith("RACE_STARTS")) {
    g_replay_count = parseInt(cmdline.split(" ")[2]);
    g_replay_rate = parseFloat(cmdline.split(" ")[3]);
    g_replay_timeout = setTimeout(
      function() {
        g_preempted = true;
        on_replay();
      },
      g_replay_length * 1000 - g_replay_timeout_epsilon);
  } else {
    console.log("Unrecognized replay message: " + cmdline);
  }
}

$(function() { setInterval(poll_as_replay, 250); });

function on_device_selection(selectq) {
  stream = document.getElementById("preview").srcObject;
  
  // If a stream is already open, stop it.
  if (stream != null) {
    stream.getTracks().forEach(function(track) {
      track.stop();
    });
  }	
	
  let device_id = selectq.find(':selected').val();
  navigator.mediaDevices.getUserMedia(
    { video: {
        deviceId: device_id,
        width: { ideal: $(window).width() },
        height: { ideal: $(window).height() }
      }
    })
  .then(stream => {
      g_recorder = new VideoCaptureFlight(stream, g_replay_length, 1000);
      document.getElementById("preview").srcObject = stream;
    });
}

$(function() {
    if (typeof(window.MediaRecorder) == 'undefined') {
      $("#replay-setup").empty()
      .append("<h3 id='reject'>This browser does not support MediaRecorder.<br/>" +
              "Please use another browser for replay.</h3>")
      .append("<p>" + navigator.userAgent + "</p>");
      return;
    }

    if (typeof(navigator.mediaDevices) == 'undefined') {
      // This happens if we're not in a secure context
      var setup = $("#replay-setup").empty()
                .append("<h3 id='reject'>Access to cameras is blocked.</h3>");
      if (window.location.protocol == 'http:') {
        var https_url = "https://" + window.location.hostname + window.location.pathname;
        setup.append("<p>You may need to switch to <a href='" +  https_url + "'>" + https_url + "</a></p>");
      }
      return;
    } else {
      navigator.mediaDevices.ondevicechange = function(event) {
        video_devices($("#device-picker :selected").prop('value'),
                      (found, options) => {
                        $("#device-picker").empty().append(options);
                        if (!found) {
                          on_setup();
                          on_device_selection($("#device-picker"));
                        }
                      });
      };
    }

    if (false) {
      // TODO Remote camera not working yet
      let id = make_viewer_id();
      $("#viewerid").text(id);
      g_remote_poller = new RemoteCamera(
        id, function(stream) {
          document.querySelector('#preview').srcObject = stream;
          // Capturing from #preview doesn't do any better
          //g_recorder = new VideoCaptureFlight(
          //  document.querySelector('#preview').captureStream(), g_replay_length, 1000);
          g_recorder = new VideoCaptureFlight(stream, g_replay_length, 1000);
        });
    } else {  // Picker for local camera
      video_devices(false, (found, options) => {
          let picker = $("#device-picker");
          picker.append(options)
                .on('input', event => {
                    on_device_selection($("#device-picker"));
                  });
          on_device_selection(picker);
        });

      $("#playback").on('ended', function(event) {
          --g_replay_count;
          console.log('End of playback; ' + g_replay_count + ' to go');
          if (g_replay_count <= 0) {
            $("#playback-background").hide('slide');
            announce_to_interior('replay-ended');
          } else {
            console.log('Replay rate of ' + g_replay_rate);
            document.querySelector("#playback").playbackRate = g_replay_rate;
            document.querySelector("#playback").play();
          }
        });
    }
});

// Posts a message to the page running in the interior iframe.
function announce_to_interior(msg) {
  document.querySelector("#interior").contentWindow.postMessage(msg, '*');
}

function on_replay() {
  // Capture these global variables before starting the asynchronous operation,
  // because they're reasonably likely to be clobbered by another queued message
  // from the server.
  var upload = g_upload_videos;
  var root = g_video_name_root;
  // If this is a replay triggered after RACE_START, make sure we don't start
  // another replay for the same heat.

  if (g_replay_timeout > 0) {
    clearTimeout(g_replay_timeout);
  }
  g_replay_timeout = 0;

  announce_to_interior('replay-started');

  g_recorder.stop(function(blob) {
      if (blob) {
        console.log('Blob of size ' + blob.size + ' and type ' + blob.type);

        if (upload && root != "") {
          let form_data = new FormData();
          form_data.append('video', blob, root + ".mkv");
          form_data.append('action', 'video.upload');
          $.ajax("action.php",
                 {type: 'POST',
                   data: form_data,
                   processData: false,
                   contentType: false,
                   success: function(data) {
                     console.log('ajax success');
                     console.log(data);
                   }
                 });
        }

        document.querySelector("#playback").src = URL.createObjectURL(blob);
        $("#playback-background").show('slide');
        console.log('Replay rate of ' + g_replay_rate);
        document.querySelector("#playback").playbackRate = g_replay_rate;
        document.querySelector("#playback").play();
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

  $("#replay-setup").hide('slide', {direction: 'down'});
  $("#playback-background").hide('slide').removeClass('hidden');
  $("#interior").removeClass('hidden');
}

function on_setup() {
  $("#replay-setup").show('slide', {direction: 'down'});
}

</script>
</head>
<body>

<iframe id="interior" class="hidden full-window">
</iframe>

<div id="playback-background" class="hidden full-window">
<video id="playback" class="full-window" autoplay muted playsinline>
</video>
</div>

<div id="replay-setup" class="full-window block_buttons">
  <?php make_banner('Replay'); ?>
  <video id="preview" autoplay muted playsinline>
  </video>

  <div id="device-picker-div">
    <select id="device-picker"></select>
  </div>

  <input type="checkbox" id="go-fullscreen" checked="checked"/>
  <label for="go-fullscreen">Change to fullscreen?</label>
  <br/>

  <label for="inner_url">URL:</label>
  <input type="text" name="inner_url" id="inner_url" size="100"
         value="<?php echo htmlspecialchars($kiosk_url, ENT_QUOTES, 'UTF-8'); ?>"/>
  <input type="button" data-enhanced="true" value="Proceed"
         onclick="on_proceed();"/>
</div>

</body>
</html>
