<?php @session_start();
// Redirects to setup page if the database hasn't yet been set up
require_once('inc/data.inc');
require_once('inc/schema_version.inc');
require_once('inc/banner.inc');
require_once('inc/authorize.inc');

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

function make_link_button($label, $link, $hidden = NULL) {
  echo "<form method='get' action='".$link."'>\n";
  if (!is_null($hidden)) {
    foreach ($hidden as $param => $value) {
      echo "<input type='hidden' name='".$param."' value='".$value."'/>\n";
    }
  }
  echo "<input type='submit' value='".$label."'/>\n";
  echo "</form>\n";
  echo "<br/>\n";
}

?><!DOCTYPE html>
<html>
<head>
<meta http-equiv="Content-Type" content="text/html; charset=UTF-8"/>
<title>Pinewood Derby Race Information</title>
<?php require('inc/stylesheet.inc'); ?>
<script type="text/javascript" src="js/jquery.js"></script>
<script type="text/javascript" src="js/modal.js"></script>
<style type="text/css">
div.index_spacer {
  height: 40px;
}

div.index_background {
  width: 100%;
}

div.index_column {
  width: 50%;
  display: inline-block;
  float: left;
}
</style>
</head>
<body>
<?php
  make_banner('', /* back_button */ false);

 $need_spacer = false;

 // This is a heuristic more than a hard rule -- when are there so many buttons that we need a second column?
 $two_columns = have_permission(SET_UP_PERMISSION);

echo "<div class='index_background'>\n";

if ($two_columns) {
  echo "<div class='index_column'>\n";
}

echo "<div class='block_buttons'>\n";

if (have_permission(SET_UP_PERMISSION)) {
  $need_spacer = true;
  make_link_button('Race Dashboard', 'coordinator.php');
  make_link_button('Kiosk Dashboard', 'kiosk-dashboard.php');
}

if (have_permission(EDIT_AWARDS_PERMISSION)) {
  $need_spacer = true;
  make_link_button('Awards Editor', 'awards-editor.php');
}

if (have_permission(PRESENT_AWARDS_PERMISSION)) {
  $need_spacer = true;
  make_link_button('Present Awards', 'awards-presentation.php');
}

if ($need_spacer) {
  $need_spacer = false;
  echo "<div class='index_spacer'>&nbsp;</div>\n";
}

if (have_permission(CHECK_IN_RACERS_PERMISSION)) {
  $need_spacer = true;
  make_link_button('Race Check-In', 'checkin.php');
}

if (have_permission(ASSIGN_RACER_IMAGE_PERMISSION)) {
  $need_spacer = true;
  make_link_button('Edit Racer Photos', 'photo-thumbs.php', array('repo' => 'head'));
  if ($schema_version > 1) {
    make_link_button('Edit Car Photos', 'photo-thumbs.php', array('repo' => 'car'));
  }
}

if ($two_columns) {
  echo "</div>\n";
  echo "</div>\n";

  echo "<div class='index_column'>\n";
  echo "<div class='block_buttons'>\n";
}

if (have_permission(SET_UP_PERMISSION)) {
  $need_spacer = true; 
  make_link_button('Set-Up', 'setup.php');
  // TODO No br
}

if ($need_spacer) {
  $need_spacer = false;
  echo "<div class='index_spacer'>&nbsp;</div>\n";
}

make_link_button('Racers On Deck', 'ondeck.php');

if (have_permission(VIEW_RACE_RESULTS_PERMISSION)) {
  make_link_button('Results By Racer', 'racer-results.php');
}

echo "<div class='index_spacer'>&nbsp;</div>\n";

if (have_permission(VIEW_AWARDS_PERMISSION)) {
  $need_spacer = true;
  make_link_button('Standings', 'standings.php');
}

if (have_permission(VIEW_RACE_RESULTS_PERMISSION)) {
  make_link_button('Exported Results', 'export.php');
}

if ($need_spacer) {
  $need_spacer = false;
  echo "<div class='index_spacer'>&nbsp;</div>\n";
}

make_link_button('About', 'about.php');

if (@$_SESSION['role']) {
  make_link_button('Log out', 'login.php', array('logout' => ''));
} else {
  make_link_button('Log in', 'login.php');
}
// TODO No <br/>

echo "</div>\n";
if (have_permission(SET_UP_PERMISSION)) {
  echo "</div>\n";
}

echo "</div>\n";
echo "</body>\n";
echo "</html>\n";
?>