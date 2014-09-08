<?php
@include("local/config-database.inc");
if (!isset($db)) {
	header('Location: setup.php');
	exit();
}

$db->setAttribute(PDO::ATTR_ERRMODE, PDO::ERRMODE_EXCEPTION);

//echo '<pre>'; print_r(PDO::getAvailableDrivers()); echo '</pre>';
// On Mac, produces array('mysql', 'sqlite')
// On Windows, produces array('mysql', 'odbc', 'sqlite')

$dbtype = $db->getAttribute(PDO::ATTR_DRIVER_NAME);
if ($dbtype == 'odbc') {
	$dbtype = 'access';
}

function sql_file_path($stem) {
  global $dbtype;
  return 'sql'.DIRECTORY_SEPARATOR.$dbtype.DIRECTORY_SEPARATOR.$stem.'.inc';
}

function read_single_row($sql, $params) {
  global $db;
  $rs = $db->prepare($sql);
  $rs->execute($params);
  $row = $rs->fetch(PDO::FETCH_NUM);
  $rs->closeCursor();
  return $row;
}

function read_single_value($sql, $params, $def = false) {
  $row = read_single_row($sql, $params);
  if ($row === false || $row[0] === null) {
	return $def;
  }

  return $row[0];
}


// The RaceInfo table is just a simple ItemKey/ItemValue association, and stores individual
// configuration settings from GPRM.  As long as there aren't any name conflicts, we can add a 
// few configuration settings of our own.
function read_raceinfo($key, $def = false) {
  return read_single_value('SELECT itemvalue FROM RaceInfo WHERE itemkey = :key',
						   array(':key' => $key), $def);
}

function read_raceinfo_boolean($key) {
  return read_raceinfo($key) ? true : false;
}

function write_raceinfo($key, $val) {
  global $db;
  $stmt = $db->prepare('SELECT COUNT(*) FROM RaceInfo WHERE itemkey = :key');
  $stmt->execute(array(':key' => $key));
  if ($stmt->fetchColumn() == 0) {
	$stmt = $db->prepare('INSERT INTO RaceInfo(itemkey, itemvalue) VALUES(:key, :value)');
  } else {
	$stmt = $db->prepare('UPDATE RaceInfo SET itemvalue = :value WHERE itemkey = :key');
  }
  $stmt->execute(array(':key' => $key,
					   ':value' => $val));
}


// GPRM sets these in "Advanced Settings", but they're not stored in the database.
// Presumably they're in the Windows registry somewhere.

function group_label() {
  return read_raceinfo('GroupLabel', 'Den');
}
function group_label_lc() {
  return strtolower(group_label());
}

function subgroup_label() {
  return read_raceinfo('SubgroupLabel', 'Subgroup');
}
function subgroup_label_lc() {
  return strtolower(subgroup_label());
}

function supergroup_label() {
  return read_raceinfo('SupergroupLabel', 'Pack');
}
function supergroup_label_lc() {
  return strtolower(supergroup_label());
}
function use_master_sched() {
  return read_raceinfo_boolean('use-master-sched');
}
function update_period() {
  return read_raceinfo('update-period', 2500);
}

function photo_directory() {
  // PHP_OS == 'Darwin'
  return read_raceinfo('photo-directory', 'C:\\racer_photos');
}

function photo_size() {
  return read_raceinfo('photo-size', '94x125');
}

function get_lane_count() {
  global $db;
  $rs = $db->query('SELECT MAX(lane), COUNT(*) FROM RaceChart');
  $row = $rs->fetch(PDO::FETCH_NUM);
  if (!$row || !isset($row[0]) || !$row[0]) {
    return read_raceinfo('lane_count', 0);
  } else {
	return $row[0];
  }
}

// Returns the highest ResultID from the RaceChart table.
// Assuming this increases monotonically, a newly scheduled
// round is detected by a new high-water resultid.  Deleting a
// schedule is undetected, but we don't really care until/unless
// there's a new schedule to replace it.
function high_water_resultid() {
  return read_single_value('SELECT MAX(resultid) FROM RaceChart', array(), 0);
}

// Returns the highest RoundID from the Rounds table.
// For ondeck, a new high-water RoundID signals a page reload.
function high_water_rounds() {
  global $db;
  $stmt = $db->query('SELECT MAX(roundid), MAX(round) FROM Rounds');
  $rs = $stmt->fetch(PDO::FETCH_NUM);
  if ($rs === false || is_null($rs[0]) || is_null($rs[1])) {
    $rs = array(0 => 0, 1 => 0);
  } 
  return array('roundid' => $rs[0],
			   'round' => $rs[1]);
}

// Returns an array value with elements for { classid => , roundid => , heat => }
// describing the currently-running round and heat.
function get_running_round() {
  global $db;
  $config = array('use_master_sched' => 0,
                  'now_racing' => 0,
				  'class' => '',
				  'roundid' => -1);  // May be overwritten

  foreach ($db->query('SELECT itemkey, itemvalue FROM RaceInfo') as $rs) {
    $key = $rs['itemkey'];
    $val = $rs['itemvalue'];
    if ($key == 'ClassID')
      $config['classid'] = $val;
    else if ($key == 'RoundID')
      $config['roundid'] = $val;
    else if ($key == 'Heat')
      $config['heat'] = $val;
    // GPRM doesn't record use-master-sched in the database; it has to
    // be set explicitly from the settings page.
    // TODO: use-master-sched key => UseMasterSched
    else if ($key == 'use-master-sched')
      $config['use_master_sched'] = $val ? 1 : 0;
    else if ($key == 'NowRacingState')
      $config['now_racing'] = $val ? 1 : 0;
  }

  $stmt = $db->prepare('SELECT round, class'
					 .' FROM Rounds'
					 .' INNER JOIN Classes'
					 .' ON Rounds.classid = Classes.classid'
					 .' WHERE roundid = :roundid'
					 .' AND Rounds.classid = :classid');
  $stmt->execute(array(':roundid' => @$config['roundid'],
					   ':classid' => @$config['classid']));
  $rs = $stmt->fetch(PDO::FETCH_ASSOC);
//  if ($rs)
 {
    $config['class'] = $rs['class'];
    $config['round'] = $rs['round'];
  }

  return $config;
}

// This HTML text will also get dynamically updated via update.js,
// so make sure any changes get made in both places!
function running_round_header($now_running, $groups_use_roundid = FALSE) {
  if (@$now_running['class']) {
    echo '<h2 class="now_running">Now racing: '
      .'<a href="#group_'.@$now_running[$groups_use_roundid ? 'roundid' : 'groupid'].'">'
      .($now_running['use_master_sched'] ? ''
        : htmlspecialchars($now_running['class'].', ', ENT_QUOTES, 'UTF-8'))
      .'Round '.@$now_running['round']
      .'</a>'
      .'</h2>'."\n";
  } else {
    echo '<h2 class="now_running">Racing has not yet begun.</h2>'."\n";
  }
}
?>