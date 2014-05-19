<?php session_start(); ?>
<?php
require_once('inc/data.inc');
require_once('inc/authorize.inc');
require_permission(SET_UP_PERMISSION);

// TODO: settings POST should be an action
// TODO: block_buttons style throughout the site?

if ($_POST) {
  write_raceinfo('use-subgroups', @$_POST['do-use-subgroups'] ? '1' : '0');
  write_raceinfo('n-den-trophies', $_POST['n-den-trophies']);
  write_raceinfo('n-pack-trophies', $_POST['n-pack-trophies']);
  write_raceinfo('xbs-award', $_POST['use-xbs'] ? $_POST['xbs-award'] : '');
  write_raceinfo('group-label', $_POST['group-label']);
  write_raceinfo('subgroup-label', $_POST['subgroup-label']);
  write_raceinfo('supergroup-label', $_POST['supergroup-label']);
  write_raceinfo('use-master-sched', @$_POST['use-master-sched'] ? '1' : '0');
  write_raceinfo('update-period', $_POST['update-period']);
  write_raceinfo('kiosk-page', $_POST['kiosk-page']);
  write_raceinfo('photo-directory', $_POST['photo_dir']);
  write_raceinfo('photo-size', $_POST['photo_width'].'x'.$_POST['photo_height']);
}
?>
<html>
<head>
<title>Pinewood Derby Race Settings</title>
<?php require('inc/stylesheet.inc'); ?>
</head>
<body>
<?php
$banner_title = 'Settings';
require('inc/banner.inc');

$nlanes = get_lane_count();
$use_subgroups = read_raceinfo_boolean('use-subgroups');
$use_xbs = read_raceinfo_boolean('xbs-award');
$xbs_award = read_raceinfo('xbs-award');
if (!$xbs_award) $xbs_award = 'Exclusively By Scout';
$use_master_sched = read_raceinfo_boolean('use-master-sched');

?>
<p>The track has <?php echo $nlanes; ?> lane(s).</p>
<div class="block_buttons">
<form method="POST">
  <div class="settings_group">
    <input id="group-label" name="group-label" type="text" value="<?php echo group_label(); ?>"/>
    <label for="group-label">Group Label</label>
    <br/>
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
    <input id="use-xbs" name="use-xbs" type="checkbox"<?php if ($use_xbs) echo ' checked="checked"';?>/>
    <label for="use-xbs">Offer "Exclusively By Scout" award?</label>
    <br/>
    <input id="xbs-award" name="xbs-award" type="text"
           value="<?php echo $xbs_award; ?>"/>
    <label for="xbs-award">"Exclusively By Scout" award name (if used)</label>
  </div>
  <br/>

  <div class="settings_group">
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
    <label for="kiosk-page">Kiosk display page:</label>
    <select name="kiosk-page">
      <optgroup>
      <?php
        function scan_kiosks($prefix, $kiosk_page) {
            $dh = @opendir(dirname(__FILE__).DIRECTORY_SEPARATOR.$prefix);
            if ($dh !== FALSE) {
                while (($entry = readdir($dh)) !== FALSE) {
                    if (substr($entry, -6) == ".kiosk") {
                        echo '<option '.($entry == $kiosk_page ? 'selected="1" ' : '')
                        .'value="'.$prefix.DIRECTORY_SEPARATOR.$entry.'">'.$entry.'</option>'."\n";
                    }
                }
                closedir($dh);
            }
        }

        $kiosk_page = read_raceinfo('kiosk-page', 'welcome.kiosk');
        scan_kiosks('kiosks', $kiosk_page);
        scan_kiosks('local'.DIRECTORY_SEPARATOR.'kiosks', $kiosk_page);
      ?>
    </optgroup>
    </select>
  </div>
  <br/>

  <div class="settings_group">
    <label for="photo_dir">Directory for racer photos:</label>
	<input id="photo_dir" name="photo_dir" type="text"
		  value="<?php echo htmlspecialchars(photo_directory()); ?>"/>
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

<table>
<?php
    if (FALSE) {
	  foreach ($db->query('SELECT itemkey, itemvalue FROM RaceInfo') as $row) {
		echo '<tr><td>'.$row['itemkey'].'</td><td>'.$row['itemvalue'].'</td></tr>'."\n";
      }
    }
?>
</table>
</body>
</html>