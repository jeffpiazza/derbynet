<?php @session_start(); 

// Login page: present a list of the possible roles, and, upon
// selection, prompt for a password and log the user in.

require_once('inc/data.inc');
require_once('inc/permissions.inc');
require_once('inc/authorize.inc');
?><!DOCTYPE html>
<html>
<head>
<meta http-equiv="Content-Type" content="text/html; charset=UTF-8"/>
<title>Please Log In</title>
<?php require('inc/stylesheet.inc'); ?>
<script type="text/javascript" src="js/jquery.js"></script>
<script type="text/javascript" src="js/modal.js"></script>
<script type="text/javascript" src="js/login.js"></script>
</head>
<body<?php if (isset($_GET['logout'])) echo ' onload="handle_logout()"'; ?>>

<?php require('inc/banner.inc'); ?>

<div class="index_background">
<div class="block_buttons">
<legend>Please choose a role:</legend>

<?php
foreach ($roles as $name => $details) {
  if (!$details['password']) {
    $logout = $name;
  } else {
    echo '<input type="button" value="'.$name.'" onclick=\'show_password_form("'.$name.'");\'/>'."\n";
  }
}
?>

<div class="login-kiosk">
<input type="button" value="Be a Kiosk" onclick="show_kiosk_form();"/>
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
  <input type="submit" value="Submit"/>
  <input type="button" value="Cancel"
      onclick='close_modal("#password_modal");'/>
</form>
</div>

<div id='kiosk_modal' class="modal_dialog hidden block_buttons">

<input type="button" value="Be a Kiosk" onclick="window.location='kiosk.php'"/>
<input type="button" value="Fullscreen Kiosk" onclick="window.location='fullscreen.php'"/>
<br/>
<input type="button" value="Cancel" onclick='close_modal("#kiosk_modal");'/>

</div>

</body>
</html>
