<?php session_start(); ?>
<?php
require_once('inc/data.inc');
require_once('inc/authorize.inc');

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

?>

<div class="block_buttons">
<form id="settings_form">
  <input type="hidden" name="action" value="write-settings"/>

  <div class="settings_group">
    <input id="n-lanes" name="n-lanes" type="number" min="0" max="20"
           value="<?php echo get_lane_count(); ?>"/>
    <label for="n-lanes">Number of lanes on the track.</label>
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
    <input id="update-period" name="update-period" type="number" min="0" max="100000"
           value="<?php echo update_period(); ?>"/>
    <label for="update-period">Page refresh period: ms. between updates.  Increase this value if necessary 
                               to reduce load on server.</label>
  </div>
  <br/>

  <div class="settings_group">
    <label for="photo_dir">Directory for racer photos:</label>
	<input id="photo_dir" name="photo_dir" type="text"
		  value="<?php echo htmlspecialchars(photo_directory(), ENT_QUOTES, 'UTF-8'); ?>"/>
    <br/>
    <?php $photosize = explode('x', photo_size()); ?>
	<label for="photo_width">Racer photo size:</label>
 	<input id="photo_width" name="photo_width" type="number" min="0" max="1000"
		  value="<?php echo $photosize[0]; ?>"/>
    <label for="photo_width">w x </label>
 	<input id="photo_height" name="photo_height" type="number" min="0" max="1000"
		  value="<?php echo $photosize[1]; ?>"/>
  </div>
  <br/>

  <input type="submit"/>
</form>
</div>

</body>
</html>