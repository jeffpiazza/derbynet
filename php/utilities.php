<?php @session_start(); ?>
<html>
<head>
<title>Race Utilities</title>
<?php require('stylesheet.inc'); ?>
<script type="text/javascript" src="jquery.js"></script>
<script type="text/javascript" src="checkin.js"></script>
<script type="text/javascript">
function handle_initaudit() {
   var xmlhttp = new XMLHttpRequest();
   xmlhttp.open("POST", g_checkin_action_url, /*async*/true);
   xmlhttp.setRequestHeader("Content-type", "application/x-www-form-urlencoded");
   xmlhttp.onreadystatechange = readystate_handler;
   xmlhttp.send("action=initaudit");
   ajax_add_request();
  }
function handle_initnumbers() {
   var xmlhttp = new XMLHttpRequest();
   xmlhttp.open("POST", g_checkin_action_url, /*async*/true);
   xmlhttp.setRequestHeader("Content-type", "application/x-www-form-urlencoded");
   xmlhttp.onreadystatechange = readystate_handler;
   xmlhttp.send("action=initnumbers");
   ajax_add_request();
  }
</script>
</head>
<body>
<?php
$banner_title = 'Utilities';
 require('banner.inc');
 require_once('authorize.inc');
?>
<div class="index_background">
<div class="block_buttons">

<input type="button" value="Init Audit Table" onclick="handle_initaudit()"/>

<input type="button" value="Reset Car Numbers" onclick="handle_initnumbers()"/>

</div>
</div>
<div id="ajax_working" class="hidden">
  <span id="ajax_num_requests">0</span> request(s) pending.
</div>
</body>
</html>