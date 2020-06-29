<?php @session_start();

require_once('inc/banner.inc');
require_once('inc/data.inc');
?><!DOCTYPE html>
<html>
<head>
<meta http-equiv="Content-Type" content="text/html; charset=UTF-8"/>
<title>Timer Log</title>
<script type="text/javascript" src="js/jquery.js"></script>
<script type="text/javascript" src="js/ajax-setup.js"></script>
<script type="text/javascript">
function poll_for_timer_log(seek, timeout) {
  $.ajax('action.php',
         {type: 'GET',
          data: {query: 'timer.log',
                 seek: seek},
          success: function(data) {
             let file_data = data.documentElement.getElementsByTagName('file-data');
             if (file_data && file_data.length > 0) {
               file_data = file_data[0];
               $("#log_text").append(
                 document.createTextNode(file_data.textContent));
               $("#log_container").scrollTop($("#log_container").scrollTop()
                                             + $("#log_text")[0].getBoundingClientRect().bottom
                                             - $("#log_container").height());
               timeout = 50;
             } else {
               timeout = 2 * timeout;
               if (timeout > 1000) {
                 timeout = 1000;
               }
             }

             let file_size_elt  = data.documentElement.getElementsByTagName('file-size');
             if (file_size_elt && file_size_elt.length > 0) {
               seek = file_size_elt[0].getAttribute('size');
             } else {
               seek = 0;
             }
             setTimeout(function() { poll_for_timer_log(seek, timeout); }, timeout);
           }
         });
}

$(function() {
    $("#log-setting-form input").on('change', function(e) {
        $.ajax("action.php",
               {type: 'POST',
                data: $("#log-setting-form").serialize()});
      });
    $("#log_container").height($(window).height() -
                               $("#log_container")[0].getBoundingClientRect().top - 5);
    poll_for_timer_log(0, 50);
  });
</script>
<?php require('inc/stylesheet.inc'); ?>
<style>
#log_container {
  float: left;
  border: 2px solid black;
  width: 50%;
  overflow-y: scroll;
}
</style>
</head>
<body>
<?php make_banner('Timer Log'); ?>
<div>&nbsp;</div>
<div id="log_container">
  <pre id="log_text">
  </pre>
</div>

<form id="log-setting-form" action="action.php">
  <input type="hidden" name="action" value="settings.write"/>
  <?php $logging_on = read_raceinfo_boolean('timer-send-logs'); ?>
  <p>Remote Logging</p>
  <input type="radio" name="timer-send-logs" id="remote-logs-on"
      value="1" <?php if ($logging_on) echo " checked"; ?>/>
  <label for="remote-logs-on">On</label>
  <br/>
  <input type="radio" name="timer-send-logs" id="remote-logs-off"
      value="0" <?php if (!$logging_on) echo " checked"; ?>/>
  <label for="remote-logs-off">Off</label>
</form>
</body>
</html>
