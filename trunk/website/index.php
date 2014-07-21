<?php @session_start(); ?>
<html>
<head>
<title>Pinewood Derby Race Information</title>
<?php require('inc/stylesheet.inc'); ?>
</head>
<body>
<?php
 $no_back_button = true;
 require('inc/banner.inc');
 require_once('inc/authorize.inc');

 $need_spacer = false;
?>
<div class="index_background">
<div class="block_buttons">

<?php if (have_permission(CHECK_IN_RACERS_PERMISSION)) { $need_spacer = true; ?>
<form method="link" action="checkin.php">
  <input type="submit" value="Race Check-In"/>
</form>
<br/>
<?php } ?>

<?php if (have_permission(ASSIGN_RACER_IMAGE_PERMISSION)) { $need_spacer = true; ?>
<form method="link" action="photo-thumbs.php">
  <input type="submit" value="Edit Racer Photos"/>
</form>
<br/>
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
  <input type="submit" value="Racer Results"/>
</form>
<br/>
 <?php } ?>

 <?php if (have_permission(COORDINATOR_PAGE_PERMISSION)) { ?>
<form method="link" action="coordinator.php">
  <input type="submit" value="Race Dashboard"/>
</form>
<br/>
 <?php } ?>

<div class="index_spacer">&nbsp;</div>

<?php if (have_permission(VIEW_AWARDS_PERMISSION)) { $need_spacer = true; ?>
<form method="link" action="awards.php">
  <input type="submit" value="Awards"/>
</form>
<br/>
<?php } ?>

<?php if (have_permission(SET_UP_PERMISSION)) { $need_spacer = true; ?>
<form method="link" action="settings.php">
  <input type="submit" value="Settings"/>
</form>
<br/>
<?php } ?>
<?php if (false) { $need_spacer = true; ?>
<form method="link" action="utilities.php">
  <input type="submit" value="Utilities"/>
</form>
<br/>
<?php } ?>

<?php
if ($need_spacer) {
  $need_spacer = false;
  echo '<div class="index_spacer">&nbsp;</div>'."\n";
}
?>

<form method="link" action="login.php">
 <?php if ($_SESSION['role']) { ?>
  <input type="hidden" name="logout" value=""/>
 <?php } ?>
  <input type="submit" value="Log <?php echo $_SESSION['role'] ? 'out' : 'in'; ?>"/>
</form>

</div>
</div>
</body>
</html>