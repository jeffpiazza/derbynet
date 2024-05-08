<?php @session_start(); 

// Login page: present a list of the possible roles, and, upon
// selection, prompt for a password and log the user in.

require_once('inc/data.inc');
require_once('inc/authorize.inc');
session_write_close();
require_once('inc/banner.inc');
require_once('inc/permissions.inc');
require_once('inc/locked.inc');
?><!DOCTYPE html>
<html>
<head>
<meta http-equiv="Content-Type" content="text/html; charset=UTF-8"/>
<title>Please Log In</title>
<?php require('inc/stylesheet.inc'); ?>
<style type="text/css">
#pw_for_password {
    display: block;
    width: 270px; /* Allow for 15px horizontal padding from global.css */
    margin-left: auto;
    margin-right: auto;
}

#banner_link {
    cursor: default;
}

#login-kiosk {
  position: fixed;
  bottom: 20px;
/*  left: 1em;
  padding: 1em; */
  width: 25em;
  left: 50%;
  margin-left: -12.5em;
}

#camera_button {
  padding-bottom: 15px;
  padding-top: 5px;
  font-size: 22px;
  height: 20px;
  width: 150px;
}
#kiosk_button {
  margin-bottom: 6px;
}
</style>
<script type="text/javascript" src="js/jquery.js"></script>
<script type="text/javascript" src="js/ajax-setup.js"></script>
<script type="text/javascript" src="js/modal.js"></script>
<script type="text/javascript" src="js/login.js"></script>
</head>
<body<?php if (isset($_GET['logout'])) echo ' onload="handle_logout()"'; ?>>

<a id="banner_link" href="<?php echo isset($_GET['all']) ? '?' : '?all'; ?>">
  <?php make_banner(''); ?>
</a>

<div class="index_background">
<div class="block_buttons">
<legend>Please choose a role:</legend>

<?php
foreach ($roles as $name => $details) {
  if (empty($name)) {
    // "Logged out" role: no name, no password
  } else if (isset($details['interactive']) && !$details['interactive'] && !isset($_GET['all'])) {
    // Ignore logins intended for robots
  } else {
    echo '<input type="button" value="'.$name.'" onclick=\'show_password_form("'.$name.'");\'/>'."\n";
  }
}
?>

<div id="login-kiosk">
<input type="button" id="kiosk_button" value="Be a Kiosk" onclick="show_kiosk_form();"/>
<a class="button_link" id="camera_button" href="camera.php">Be a Camera</a>
</div>

<?php
if (@$_SESSION['role']) {
?>
<br/>
<input type="button" value="Log out" onclick='handle_logout();'/>
<?php
}
?>

</div>
</div>

<div id='password_modal' class="modal_dialog hidden block_buttons">
<form>
  <input type="hidden" name="name" id="name_for_password" value=""/>
  <p>Enter password:</p>
  <p><input type="password" name="pw" id="pw_for_password"/></p>
  <?php if (!locked_settings()) { ?>
  <p>&nbsp;</p>
  <p>Don't remember setting a password?  Consult the documentation.</p>
  <?php } ?>
  <input type="submit" value="Submit"/>
  <input type="button" value="Cancel"
      onclick='close_modal("#password_modal");'/>
</form>
</div>

<div id='kiosk_modal' class="modal_dialog hidden block_buttons">

<input type="button" value="Be a Kiosk" onclick="window.location='kiosk.php'"/>
<input type="button" value="Fullscreen Kiosk" onclick="window.location='fullscreen.php'"/>
<input type="button" value="Replay Kiosk" onclick="window.location='replay.php'"/>
<br/>
<input type="button" value="Cancel" onclick='close_modal("#kiosk_modal");'/>

</div>

</body>
</html>
