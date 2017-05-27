<?php @session_start();
// Redirects to setup page if the database hasn't yet been set up
require_once('inc/data.inc');
require_once('inc/schema_version.inc');

// This first database access is surrounded by a try/catch in order to catch
// broken/corrupt databases (e.g., sqlite pointing to a file that's not actually
// a database).  The pdo may get created OK, but then fail on the first attempt
// to access.
try {
  $schema_version = schema_version();
} catch (PDOException $p) {
  $_SESSION['setting_up'] = 1;
  header('Location: setup.php');
  exit();
}
?><!DOCTYPE html>
<html>
<head>
<meta http-equiv="Content-Type" content="text/html; charset=UTF-8"/>
<title>Pinewood Derby Race Information</title>
<?php require('inc/stylesheet.inc'); ?>
<script type="text/javascript" src="js/jquery.js"></script>
<script type="text/javascript" src="js/modal.js"></script>
</head>
<body>
<?php
 $no_back_button = true;
 require('inc/banner.inc');
 require_once('inc/authorize.inc');

 $need_spacer = false;

 // This is a heuristic more than a hard rule -- when are there so many buttons that we need a second column?
 $two_columns = have_permission(SET_UP_PERMISSION);
?>
<div class="index_background">

<?php if ($two_columns) { ?>
<div class="index_column">
<?php } ?>

<div class="block_buttons">

<?php if (have_permission(SET_UP_PERMISSION)) { $need_spacer = true; ?>
<form method="link" action="coordinator.php">
  <input type="submit" value="Race Dashboard"/>
</form>
<br/>
<form method="link" action="kiosk-dashboard.php">
  <input type="submit" value="Kiosk Dashboard"/>
</form>
<br/>
<?php } ?>

<?php if (have_permission(EDIT_AWARDS_PERMISSION)) { $need_spacer = true; ?>
<form method="link" action="awards-editor.php">
  <input type="submit" value="Awards Editor"/>
</form>
<br/>
<?php } ?>

<?php if (have_permission(PRESENT_AWARDS_PERMISSION)) { $need_spacer = true; ?>
<form method="link" action="awards-presentation.php">
  <input type="submit" value="Present Awards"/>
</form>
<br/>
<?php } ?>
 
<?php
if ($need_spacer) {
  $need_spacer = false;
  echo '<div class="index_spacer">&nbsp;</div>'."\n";
}
?>

<?php if (have_permission(CHECK_IN_RACERS_PERMISSION)) { $need_spacer = true; ?>
<form method="link" action="checkin.php">
  <input type="submit" value="Race Check-In"/>
</form>
<br/>
<?php } ?>

<?php if (have_permission(ASSIGN_RACER_IMAGE_PERMISSION)) { $need_spacer = true; ?>
<form method="get" action="photo-thumbs.php">
  <input type="hidden" name="repo" value="head"/>
  <input type="submit" value="Edit Racer Photos"/>
</form>
<br/>

<?php if ($schema_version > 1) { ?>
<form method="get" action="photo-thumbs.php">
  <input type="hidden" name="repo" value="car"/>
  <input type="submit" value="Edit Car Photos"/>
</form>
<br/>
<?php } ?>
<?php } ?>

<?php if ($two_columns) { ?>
</div>
</div>

<div class="index_column">
<div class="block_buttons">
<?php } ?>

<?php if (have_permission(SET_UP_PERMISSION)) { $need_spacer = true; ?>

<form method="link" action="setup.php">
  <input type="submit" value="Set-Up"/>
</form>
                                                
<?php } ?>

<?php
if ($need_spacer) {
  $need_spacer = false;
  echo '<div class="index_spacer">&nbsp;</div>'."\n";
}
?>

<form method="link" action="ondeck.php">
  <input type="submit" value="Racers On Deck"/>
</form>
<br/>

 <?php if (have_permission(VIEW_RACE_RESULTS_PERMISSION)) { ?>
<form method="link" action="racer-results.php">
  <input type="submit" value="Results By Racer"/>
</form>
<br/>
 <?php } ?>

<div class="index_spacer">&nbsp;</div>

<?php if (have_permission(VIEW_AWARDS_PERMISSION)) { $need_spacer = true; ?>
<form method="link" action="standings.php">
  <input type="submit" value="Standings"/>
</form>
<br/>
<?php } ?>

 <?php if (have_permission(VIEW_RACE_RESULTS_PERMISSION)) { ?>
<form method="link" action="export.php">
  <input type="submit" value="Exported Results"/>
</form>
<br/>
 <?php } ?>

<?php
if ($need_spacer) {
  $need_spacer = false;
  echo '<div class="index_spacer">&nbsp;</div>'."\n";
}
?>

<form method="link" action="about.php">
  <input type="submit" value="About"/>
</form>
<br/>

<form method="link" action="login.php">
 <?php if (@$_SESSION['role']) { ?>
  <input type="hidden" name="logout" value=""/>
 <?php } ?>
  <input type="submit" value="Log <?php echo $_SESSION['role'] ? 'out' : 'in'; ?>"/>
</form>

</div>
<?php if (have_permission(SET_UP_PERMISSION)) { ?>
</div>
<?php } ?>
</div>
</body>
</html>