<?php session_start();
require_once('inc/data.inc');
require_once('inc/banner.inc');
require_once('inc/authorize.inc');
require_once('inc/photo-config.inc');
require_once('inc/locked.inc');
require_once('inc/default-database-directory.inc');
require_once('inc/name-mangler.inc');

require_permission(SET_UP_PERMISSION);
?><!DOCTYPE html>
<html>
<head>
<meta http-equiv="Content-Type" content="text/html; charset=UTF-8"/>
<title>Pinewood Derby Race Settings</title>
<?php require('inc/stylesheet.inc'); ?>
<link rel="stylesheet" type="text/css" href="css/jquery.mobile-1.4.2.css"/>
<link rel="stylesheet" type="text/css" href="css/chooser.css"/>
<link rel="stylesheet" type="text/css" href="css/settings.css"/>
<script type="text/javascript" src="js/jquery.js"></script>
<script type="text/javascript" src="js/mobile-init.js"></script>
<script type="text/javascript" src="js/jquery.mobile-1.4.2.min.js"></script>
<script type="text/javascript" src="js/modal.js"></script>
<script type="text/javascript" src="js/dashboard-ajax.js"></script>
<script type="text/javascript" src="js/chooser.js"></script>
<script type="text/javascript" src="js/settings.js"></script>
<script type="text/javascript">
// Returns a string identifying the directory at which browsing for photo
// directories should start.  See js/settings.js' browse_for_photo_directory().
function photo_directory_base() {
  <?php
  if (isset($db_connection_string) && substr($db_connection_string, 0, 7) == 'sqlite:') {
    echo 'return '.json_encode(substr($db_connection_string, 7)).';';
  } else {
    $default_path = default_database_directory();
    if (!empty($default_path)) {
      echo 'return '.json_encode($default_path.DIRECTORY_SEPARATOR).';';
    } else {
      echo 'return "";';
    }
  }
  ?>
}
</script>
</head>
<body>
<?php
make_banner('Settings', 'setup.php');

$use_subgroups = read_raceinfo_boolean('use-subgroups');
$use_xbs = read_raceinfo_boolean('xbs-award');
$xbs_award = read_raceinfo('xbs-award');
if (!$xbs_award) $xbs_award = 'Exclusively By Scout';
$use_master_sched = read_raceinfo_boolean('use-master-sched');
$show_racer_photos = read_raceinfo_boolean('show-racer-photos');
$show_car_photos_on_deck = read_raceinfo_boolean('show-cars-on-deck');
$show_racer_photos_rr = read_raceinfo_boolean('show-racer-photos-rr');
$show_car_photos_rr = read_raceinfo_boolean('show-car-photos-rr');
$locked_settings = locked_settings();
$name_style = read_raceinfo('name-style', FULL_NAME);
?>

<div class="block_buttons">
<form id="settings_form">
  <input type="hidden" name="action" value="settings.write"/>

  <div class="settings_group">
    <div class="settings_group_image">
      <img src="img/settings-timer.png"/>
    </div>

    <div class="settings_group_settings">
    <input type="hidden" name="with-gprm-checkbox" value="yes"/>
      <p>
        <input id="with-gprm" name="with-gprm" data-enhanced="true"
                type="checkbox"<?php if (with_gprm()) echo ' checked="checked"';?>/>
        <label>Using Grand Prix Race Manager (for timer control, etc.)?</label>
      </p>
      <p>
        <input id="n-lanes" name="n-lanes" type="number" min="0" max="20"
               data-enhanced="true"
               value="<?php echo get_lane_count(); ?>"/>
        <label for="n-lanes">Number of lanes on the track.</label>
      </p>
<p>
    <input type="hidden" id="unused-lane-mask" name="unused-lane-mask"
           value="<?php echo read_raceinfo('unused-lane-mask', 0); ?>"/>
Lanes available for scheduling:</p>
<p>
<span id="lanes-in-use">
</span>
</p>
      <p>
        <input id="track-length" name="track-length" type="number" data-enhanced="true"
               value="<?php echo read_raceinfo('track-length', 40); ?>"/>
        <label for="track-length">Track length (in feet)</label>
      </p>
    </div>
  </div>

  <div class="settings_group">
    <div class="settings_group_image">
      <img src="img/settings-groups.png"/>
    </div>
    <div class="settings_group_settings">
      <p>
        <input id="supergroup-label" name="supergroup-label" type="text" data-enhanced="true"
               value="<?php echo supergroup_label(); ?>"/>
        <label for="supergroup-label">Super-Group Label</label>
      </p>
      <p>
        <input id="group-label" name="group-label" type="text" data-enhanced="true" value="<?php echo group_label(); ?>"/>
        <label for="group-label">Group Label</label>
      </p>
      <p>
        <input type="hidden" name="do-use-subgroups-checkbox" value="yes"/>
        <input id="use-subgroups" name="do-use-subgroups" data-enhanced="true" type="checkbox"<?php
            if ($use_subgroups) echo ' checked="checked"';?>/>
        <label>Use subgroups?</label>
      </p>
      <p>
        <input id="subgroup-label" name="subgroup-label" type="text" data-enhanced="true"
               value="<?php echo subgroup_label(); ?>"/>
        <label for="subgroup-label">Subgroup Label</label>
      </p>

      <p>Show racer names as:<br/>
        <input type="radio" name="name-style" value="0" id="name-style-0" data-role="none"<?php
        echo $name_style == FULL_NAME ? ' checked="checked"' : '';
        ?>/><label for="name-style-0" data-enhanced="true">First name and last name</label><br/>

        <input type="radio" name="name-style" value="1" id="name-style-1" data-role="none"<?php
        echo $name_style == FIRST_NAME_LAST_INITIAL ? ' checked="checked"' : '';
        ?>/><label for="name-style-1">First name and last initial</label>
      </p>
    </div>
  </div>

  <div class="settings_group">
    <div class="settings_group_image">
      <img src="img/settings-gold-medal.png"/>
    </div>

    <div class="settings_group_settings">
      <p>
        <input id="n-pack" name="n-pack-trophies" type="number" min="0" max="20" data-enhanced="true"
               value="<?php echo read_raceinfo('n-pack-trophies', 3); ?>"/>
        <label for="n-pack">Number of speed trophies at the <?php echo supergroup_label_lc(); ?> level</label>
      </p>
      <p>
        <input id="n-den" name="n-den-trophies" type="number" min="0" max="20" data-enhanced="true"
               value="<?php echo read_raceinfo('n-den-trophies', 3); ?>"/>
        <label for="n-den">Number of speed trophies per <?php echo group_label_lc(); ?></label>
      </p>
      <p>
        <input id="n-rank" name="n-rank-trophies" type="number" min="0" max="20" data-enhanced="true"
               value="<?php echo read_raceinfo('n-rank-trophies', 0); ?>"/>
        <label for="n-pack">Number of speed trophies per <?php echo subgroup_label_lc(); ?></label>
      </p>
      <p>
        <input type="hidden" name="use-xbs-checkbox" value="yes"/>
        <input id="use-xbs" name="use-xbs" data-enhanced="true"
                type="checkbox"<?php if ($use_xbs) echo ' checked="checked"';?>/>
        <label>Offer "Exclusively By Scout" award?</label>
      </p>
      <p>
        <input id="xbs-award" name="xbs-award" type="text" data-enhanced="true"
               value="<?php echo $xbs_award; ?>"/>
        <label for="xbs-award">"Exclusively By Scout" award name (if used)</label>
      </p>
    </div>
  </div>

<?php
function photo_settings($category, $photo_dir_id, $photo_dir_value, $photo_size_prefix) {
    if (!locked_settings()) {
      echo "<p>\n";
      echo '<label for="'.$photo_dir_id.'">Directory for '.$category.' photos:</label>'."\n";
      echo '<input id="'.$photo_dir_id.'" name="'.$photo_dir_id.'" type="text" data-enhanced="true"'
           .' size="50"'
		   .' value="'.htmlspecialchars($photo_dir_value, ENT_QUOTES, 'UTF-8').'"/>'."\n";
      echo '<span id="'.$photo_dir_id.'_icon" class="status_icon"></span>'."\n";
      echo '</p>';
      echo '<p class="photo_dir_status_message" id="'.$photo_dir_id.'_message"></p>';
      echo '<p>';
      echo '<span class="photo_dir_choose"><input type="button" value="Browse"'
          .' data-enhanced="true"'
          .' onclick="browse_for_photo_directory(\'#'.$photo_dir_id.'\')"/>'
          .'</span>'."\n";
      echo "</p>\n";
    }
}
?>

  <div class="settings_group">
    <div class="settings_group_image">
      <img src="img/settings-photos.png"/>
    </div>

    <div class="settings_group_settings">
      <p>
        <input type="hidden" name="show-racer-photos-checkbox" value="yes"/>
        <input id="show-racer-photos" name="show-racer-photos" data-enhanced="true"
               type="checkbox"<?php if ($show_racer_photos) echo ' checked="checked"';?>/>
        <label>Show racer photos on main racing board</label>
      </p>
      <p>
        <input type="hidden" name="show-car-photos-on-deck-checkbox" value="yes"/>
        <input id="show-car-photos-on-deck" name="show-car-photos-on-deck" data-enhanced="true"
               type="checkbox"<?php if ($show_car_photos_on_deck) echo ' checked="checked"';?>/>
        <label>Show car photos in on-deck display</label>
      </p>
      <p>
        <input type="hidden" name="show-racer-photos-rr-checkbox" value="yes"/>
        <input id="show-racer-photos-rr" name="show-racer-photos-rr" data-enhanced="true"
               type="checkbox"<?php if ($show_racer_photos_rr) echo ' checked="checked"';?>/>
        <label>Show racer photos in racer-results display</label>
      </p>
      <p>
        <input type="hidden" name="show-car-photos-rr-checkbox" value="yes"/>
        <input id="show-car-photos-rr" name="show-car-photos-rr" data-enhanced="true"
               type="checkbox"<?php if ($show_car_photos_rr) echo ' checked="checked"';?>/>
        <label>Show car photos in racer-results display</label>
      </p>

      <?php photo_settings('racer', 'photo-dir', photo_directory(), 'photo'); ?>
      <?php photo_settings('car', 'car-photo-dir', car_photo_directory(), 'car-photo'); ?>
    </div>
  </div>

  <div class="settings_group">
    <div class="settings_group_image">
      <img src="img/settings-rulebook.png"/>
    </div>
    <div class="settings_group_settings">
      <p>
        <input type="hidden" name="drop-slowest-checkbox" value="yes"/>
        <input id="drop-slowest" name="drop-slowest" data-enhanced="true" type="checkbox"<?php 
            if (read_raceinfo_boolean('drop-slowest')) echo ' checked="checked"';?>/>
        <label>Drop each racer's slowest heat?</label>
      </p>
      <p>
        <input type="hidden" name="use-master-sched-checkbox" value="yes"/>
        <input id="use-master-sched" name="use-master-sched" data-enhanced="true" type="checkbox"<?php
            if ($use_master_sched) echo ' checked="checked"';?>/>
        <label>Interleave heats from different <?php echo group_label_lc(); ?>s</label>
      </p>
      <p>
        <input type="hidden" name="use-points-checkbox" value="yes"/>
        <input id="use-points" name="use-points" data-enhanced="true" type="checkbox"<?php 
            if (read_raceinfo_boolean('use-points')) echo ' checked="checked"';?>/>
        <label>Race by points (place) instead of by times?</label>
      </p>
    </div>
  </div>

  <input data-enhanced="true" type="submit"/>
</form>
</div>

<?php require('inc/chooser.inc'); ?>

</body>
</html>