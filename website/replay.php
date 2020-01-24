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

var g_remote_poller;
var g_recorder;

var g_replay_count;
var g_replay_rate;

function handle_replay_message(cmdline) {
  if (cmdline.startsWith("HELLO")) {
  } else if (cmdline.startsWith("TEST")) {
    g_replay_count = parseInt(cmdline.split(" ")[2]);
    g_replay_rate = parseFloat(cmdline.split(" ")[3]);
    on_replay();
  } else if (cmdline.startsWith("START")) {
    g_video_name_root = cmdline.substr(6).trim();
  } else if (cmdline.startsWith("REPLAY")) {
    // REPLAY skipback showings rate
    //  skipback and rate are ignored, but showings we can honor
    // (Must be exactly one space between fields:)
    g_replay_count = parseInt(cmdline.split(" ")[2]);
    g_replay_rate = parseFloat(cmdline.split(" ")[3]);
    on_replay();
  } else if (cmdline.startsWith("CANCEL")) {
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
  navigator.mediaDevices.getUserMedia({ video: { deviceId: device_id } })
  .then(stream => {
      g_recorder = new VideoCaptureFlight(stream, 4, 1000);
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
          //  document.querySelector('#preview').captureStream(), 4, 1000);
          g_recorder = new VideoCaptureFlight(stream, 4, 1000);
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
  announce_to_interior('replay-started');
  g_recorder.stop(function(blob) {
      if (blob) {
        console.log('Blob of size ' + blob.size + ' and type ' + blob.type);

        if (g_upload_videos && g_video_name_root != "") {
          let form_data = new FormData();
          form_data.append('video', blob, g_video_name_root + ".mkv");
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
