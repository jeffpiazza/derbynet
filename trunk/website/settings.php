<?php session_start(); ?>
<?php
require_once('inc/data.inc');
require_once('inc/authorize.inc');
require_once('inc/photo-config.inc');

require_permission(SET_UP_PERMISSION);
?>
<html>
<head>
<meta http-equiv="Content-Type" content="text/html; charset=UTF-8"/>
<title>Pinewood Derby Race Settings</title>
<?php require('inc/stylesheet.inc'); ?>
<script type="text/javascript" src="js/jquery.js"></script>
<script type="text/javascript" src="js/settings.js"></script>
</head>
<body>
<?php
$banner_title = 'Settings';
require('inc/banner.inc');

$use_subgroups = read_raceinfo_boolean('use-subgroups');
$use_xbs = read_raceinfo_boolean('xbs-award');
$xbs_award = read_raceinfo('xbs-award');
if (!$xbs_award) $xbs_award = 'Exclusively By Scout';
$use_master_sched = read_raceinfo_boolean('use-master-sched');
$show_racer_photos = read_raceinfo_boolean('show-racer-photos');
?>

<div class="block_buttons">
<form id="settings_form">
  <input type="hidden" name="action" value="write-settings"/>

  <div class="settings_group">
    <input type="hidden" name="with-gprm-checkbox" value="yes"/>
    <input id="with-gprm" name="with-gprm" type="checkbox"<?php if (with_gprm()) echo ' checked="checked"';?>/>
  <label for="with-gprm">Using Grand Prix Race Manager (for timer control, etc.)?</label>
  </div>

  <div class="settings_group">
    <input id="n-lanes" name="n-lanes" type="number" min="0" max="20"
           value="<?php echo get_lane_count(); ?>"/>
    <label for="n-lanes">Number of lanes on the track.</label>
    <br/>
    <input id="track-length" name="track-length" type="number" value="<?php echo read_raceinfo('track-length', 40); ?>"/>
    <label for="track-length">Track length (in feet)</label>
  </div>

  <div class="settings_group">
    <input type="hidden" name="drop-slowest-checkbox" value="yes"/>
    <input id="drop-slowest" name="drop-slowest" type="checkbox"<?php 
        if (read_raceinfo_boolean('drop-slowest')) echo ' checked="checked"';?>/>
    <label for="drop-slowest">Drop each racer's slowest heat?</label>
  </div>

  <div class="settings_group">
    <input id="group-label" name="group-label" type="text" value="<?php echo group_label(); ?>"/>
    <label for="group-label">Group Label</label>
    <br/>
    <input type="hidden" name="do-use-subgroups-checkbox" value="yes"/>
    <input id="use-subgroups" name="do-use-subgroups" type="checkbox"<?php if ($use_subgroups) echo ' checked="checked"';?>/>
    <label for="use-subgroups">Use subgroups?</label>
    <br/>
    <input id="subgroup-label" name="subgroup-label" type="text" value="<?php echo subgroup_label(); ?>"/>
    <label for="subgroup-label">Subgroup Label</label>
    <br/>
    <input id="supergroup-label" name="supergroup-label" type="text" value="<?php echo supergroup_label(); ?>"/>
    <label for="supergroup-label">Super-Group Label</label>
  </div>

  <br/>
  <div class="settings_group">
    <input id="n-den" name="n-den-trophies" type="number" min="0" max="20"
           value="<?php echo read_raceinfo('n-den-trophies', 3); ?>"/>
    <label for="n-den">Number of trophies per <?php echo group_label_lc(); ?></label>
    <br/>
    <input id="n-pack" name="n-pack-trophies" type="number" min="0" max="20"
           value="<?php echo read_raceinfo('n-pack-trophies', 3); ?>"/>
    <label for="n-pack">Number of trophies for the <?php echo supergroup_label_lc(); ?></label>
  </div>
  <br/>

  <div class="settings_group">
    <input type="hidden" name="use-xbs-checkbox" value="yes"/>
    <input id="use-xbs" name="use-xbs" type="checkbox"<?php if ($use_xbs) echo ' checked="checked"';?>/>
    <label for="use-xbs">Offer "Exclusively By Scout" award?</label>
    <br/>
    <input id="xbs-award" name="xbs-award" type="text"
           value="<?php echo $xbs_award; ?>"/>
    <label for="xbs-award">"Exclusively By Scout" award name (if used)</label>
  </div>
  <br/>

  <div class="settings_group">
    <input type="hidden" name="use-master-sched-checkbox" value="yes"/>
    <input id="use-master-sched" name="use-master-sched" type="checkbox"<?php if ($use_master_sched) echo ' checked="checked"';?>/>
    <label for="use-master-sched">Use master schedules</label>
  </div>
  <br/>

  <div class="settings_group">
    <input type="hidden" name="show-racer-photos-checkbox" value="yes"/>
    <input id="show-racer-photos" name="show-racer-photos"
           type="checkbox"<?php if ($show_racer_photos) echo ' checked="checked"';?>/>
    <label for="show-racer-photos">Show racer photos on main racing board</label>
    <br/>
    <label for="photo-dir">Directory for racer photos:</label>
	<input id="photo-dir" name="photo-dir" type="text"
		  value="<?php echo htmlspecialchars(photo_directory(), ENT_QUOTES, 'UTF-8'); ?>"/>
    <br/>
    <?php $photosize = explode('x', photo_size()); ?>
	<label for="photo-width">Racer photo size:</label>
 	<input id="photo-width" name="photo-width" type="number" min="0" max="1000"
		  value="<?php echo $photosize[0]; ?>"/>
    <label for="photo-height">w x </label>
 	<input id="photo-height" name="photo-height" type="number" min="0" max="1000"
		  value="<?php echo $photosize[1]; ?>"/>
    <br/>
  </div>
  <br/>

  <div class="settings_group">
    <label for="car-photo-dir">Directory for car photos:</label>
	<input id="car-photo-dir" name="car-photo-dir" type="text"
		  value="<?php echo htmlspecialchars(photo_directory(), ENT_QUOTES, 'UTF-8'); ?>"/>
    <br/>
    <?php $car_photosize = explode('x', car_photo_size()); ?>
	<label for="car-photo_width">Car photo size:</label>
 	<input id="car-photo-width" name="car-photo-width" type="number" min="0" max="1000"
		  value="<?php echo $car_photosize[0]; ?>"/>
    <label for="car-photo-height">w x </label>
 	<input id="car-photo-height" name="car-photo-height" type="number" min="0" max="1000"
		  value="<?php echo $car_photosize[1]; ?>"/>
    <br/>
  </div>
  <br/>

  <input type="submit"/>
</form>
</div>

</body>
</html>