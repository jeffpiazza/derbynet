<?php @session_start();
require_once('inc/authorize.inc');
require_permission(SET_UP_PERMISSION);
?>
<html>
<head>
<meta http-equiv="Content-Type" content="text/html; charset=UTF-8"/>
<title>Web Race Manager Set-Up</title>
<?php require('inc/stylesheet.inc'); ?>
<link rel="stylesheet" type="text/css" href="css/jquery.mobile-1.4.2.css"/>
<link rel="stylesheet" type="text/css" href="css/coordinator.css"/>
<script type="text/javascript" src="js/jquery.js"></script>
<script type="text/javascript" src="js/jquery-ui-1.10.4.min.js"></script>
<script type="text/javascript">
// We're using jQuery Mobile for its nice mobile/tablet-friendly UI
// elements.  By default, jQM also wants to hijack page loading
// functionality, and perform page loads via ajax, a "service" we
// don't really want.  Fortunately, it's possible to turn that stuff
// off.
$(document).bind("mobileinit", function() {
                                   $.extend($.mobile, {
                                         ajaxEnabled: false
                                         });
                                 });
</script>
<!-- For flipswitch and select elements: -->
<script type="text/javascript" src="js/jquery.mobile-1.4.2.min.js"></script>
<script type="text/javascript" src="js/setup.js"></script>
</head>
<body>
<?php $banner_title = 'Set-Up'; require('inc/banner.inc'); ?>

<h3>Operating Environment</h3>
<pre><?php
echo 'PHP_OP = ';var_dump(PHP_OS);
echo 'php_uname() = '; var_dump(php_uname());
echo 'php_uname("s") = '; var_dump(php_uname('s'));
?>
</pre>
<h3>Current Configuration File</h3>
<?php

$local_config_inc = 'local/config-database.inc';

if (file_exists($local_config_inc)) {
  echo "<pre>\n";
  echo htmlspecialchars(file_get_contents($local_config_inc,
                                          /* use_include_path */ true),
                        ENT_QUOTES, 'UTF-8');
  echo "</pre>\n";
} else {
  echo "<p>You do not yet have a local configuration file.</p>\n";
  if (!is_dir('local')) {
    echo "<p>You need to create a 'local' directory, and make it writable.</p>\n";
  } else if (!is_writable('local')) {
    echo "<p>The 'local' directory exists, but isn't writable.</p>\n";
  }
}
?>
<div class="block_buttons">
    <input type="button" data-enhanced="true" value="Configure" onclick="show_choose_database_modal()"/><br/>
</div>

<div id="choose_database_modal" class="modal_dialog hidden block_buttons">
  <form>
    <input type="hidden" name="action" value="setup"/>
    <label for="connection_string">Database connection string:</label>
    <input type="text" name="connection_string" id="connection_string_field"/>
    <label for="dbuser">Database user name:</label>
    <input type="text" name="dbuser" id="username_field"/>
    <label for="dbpass">Database password:</label>
    <input type="text" name="dbpass" id="password_field"/>

    <input type="submit" data-enhanced="true" value="Submit"/>
    <input type="button" data-enhanced="true" value="Cancel"
      onclick='close_modal("#choose_database_modal");'/>
  </form>
</div>

<div id='modal_background'></div>

</body>
</html>