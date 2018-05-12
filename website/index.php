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

function make_link_button($label, $link, $permission, $button_class) {
  if (have_permission($permission)) {
    echo "<a class='button_link ".$button_class."' href='".$link."'>".$label."</a>\n";
    echo "<br/>\n";
    return true;
  } else {
    return false;
  }
}

function make_spacer_if($cond) {
  if ($cond) {
    echo "<div class='index_spacer'>&nbsp;</div>\n";
  }
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

.block_buttons a.button_link {
  width: 238px;
  height: 30px;
}

.block_buttons a.button_link.before_button,
.block_buttons input.before_button[type='submit'] {
  color: #ffffcc;
}
.block_buttons a.button_link.during_button,
.block_buttons input.during_button[type='submit'] {
  color: #ddffdd;
}
.block_buttons a.button_link.after_button,
.block_buttons input.after_button[type='submit'] {
  color: #ddddff;
}
.block_buttons a.button_link.other_button,
.block_buttons input.other_button[type='submit'] {
  color: #ffddff;
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

// *********** Before ***************
$need_spacer = make_link_button('Set-Up', 'setup.php', SET_UP_PERMISSION, 'before_button');
$need_spacer = make_link_button('Race Check-In', 'checkin.php', CHECK_IN_RACERS_PERMISSION, 'before_button') || $need_spacer;
$need_spacer = make_link_button('Edit Racer Photos', 'photo-thumbs.php?repo=head', ASSIGN_RACER_IMAGE_PERMISSION, 'before_button') || $need_spacer;
if ($schema_version > 1) {
  $need_spacer = make_link_button('Edit Car Photos', 'photo-thumbs.php?repo=car', ASSIGN_RACER_IMAGE_PERMISSION, 'before_button') || $need_spacer;
}

make_spacer_if($need_spacer);

// *********** During ***************
$need_spacer = make_link_button('Race Dashboard', 'coordinator.php', SET_UP_PERMISSION, 'during_button');
$need_spacer = make_link_button('Kiosk Dashboard', 'kiosk-dashboard.php', SET_UP_PERMISSION, 'during_button') || $need_spacer;
$need_spacer = make_link_button('Judging', 'judging.php', JUDGING_PERMISSION, 'during_button') || $need_spacer;

// end first column default set-up

if ($two_columns) {
  echo "</div>\n";  // block_buttons
  echo "</div>\n";  // index_column

  echo "<div class='index_column'>\n";
  echo "<div class='block_buttons'>\n";
}

// *********** During, part 2 ***************
$need_spacer = make_link_button('Racers On Deck', 'ondeck.php', -1, 'during_button') || $need_spacer;
$need_spacer = make_link_button('Results By Racer', 'racer-results.php', VIEW_RACE_RESULTS_PERMISSION, 'during_button')
    || $need_spacer;

make_spacer_if($need_spacer);

// *********** After ***************
$need_spacer = make_link_button('Present Awards', 'awards-presentation.php', PRESENT_AWARDS_PERMISSION, 'after_button');
$need_spacer = make_link_button('Standings', 'standings.php', VIEW_AWARDS_PERMISSION, 'after_button') || $need_spacer;
$need_spacer = make_link_button('Exported Results', 'export.php', VIEW_RACE_RESULTS_PERMISSION, 'after_button') || $need_spacer;

make_spacer_if($need_spacer);

// *********** Other ***************
make_link_button('About', 'about.php', -1, 'other_button');

if (@$_SESSION['role']) {
  make_link_button('Log out', 'login.php?logout', -1, 'other_button');
} else {
  make_link_button('Log in', 'login.php', -1, 'other_button');
}

echo "</div>\n";  // block_buttons
if ($two_columns) {
  echo "</div>\n";  // index_column
}

echo "</div>\n";  // index_background
echo "</body>\n";
echo "</html>\n";
?>