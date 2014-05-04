<?php
  //$conn = odbc_connect('DSN=gprm;ReadOnly=YES;Exclusive=NO','','');
$conn = odbc_connect('DSN=gprm;Exclusive=NO','','');
if (!$conn) {
  echo '<p>Connection by System DSN failed: '.odbc_error().' '.odbc_errormsg().'.</p>';
  exit();
}

function read_single_value($sql, $def = false) {
  global $conn;
  $rs = odbc_exec($conn, $sql);
  if (odbc_fetch_row($rs)) {
	return odbc_result($rs, 1);
  }
  return $def;
}


// The RaceInfo table is just a simple ItemKey/ItemValue association, and stores individual
// configuration settings from GPRM.  As long as there aren't any name conflicts, we can add a 
// few configuration settings of our own.
function read_raceinfo($key, $def = false) {
  return read_single_value('select ItemValue from RaceInfo where ItemKey = \''.$key.'\'', $def);
}

function read_raceinfo_boolean($key) {
  return read_raceinfo($key) ? true : false;
}

function write_raceinfo($key, $val) {
  global $conn;
  odbc_exec($conn, 'update RaceInfo set ItemValue = \''.$val.'\' where ItemKey = \''.$key.'\'');
  $rs = odbc_exec($conn, 'select ItemValue from RaceInfo where ItemKey = \''.$key.'\'');
  if (!odbc_fetch_row($rs)) {
    odbc_exec($conn, 'insert into RaceInfo(ItemKey, ItemValue) values (\''.$key.'\', \''.$val.'\')');
  }
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
function kiosk_page() {
  return read_raceinfo('kiosk-page', 'welcome.kiosk');
}

function photo_directory() {
  return read_raceinfo('photo-directory', 'C:\\racer_photos');
}

function photo_size() {
  return read_raceinfo('photo-size', '94x125');
}

function get_lane_count() {
  global $conn;
  $rs = odbc_exec($conn, 'select max(lane), count(*) from racechart');
  if ($rs === FALSE) {
    return odbc_errormsg($conn);
  }
  if (odbc_fetch_row($rs))
    return odbc_result($rs, 1);
  else
    return -1;
}

// Returns the highest ResultID from the RaceChart table.
// Assuming this increases monotonically, a newly scheduled
// round is detected by a new high-water resultid.  Deleting a
// schedule is undetected, but we don't really care until/unless
// there's a new schedule to replace it.
function high_water_resultid() {
  return read_single_value('select max(ResultID) from RaceChart', 0);
}

// Returns the highest RoundID from the Rounds table.
// For ondeck, a new high-water RoundID signals a page reload.
function high_water_rounds() {
  global $conn;
  $rs = odbc_exec($conn, 'select max(RoundID), max(Round) from Rounds');
  odbc_fetch_row($rs);
  return array('roundid' => odbc_result($rs, 1),
			   'round' => odbc_result($rs, 2));
}

// Returns an array value with elements for { classid => , roundid => , heat => }
// describing the currently-running round and heat.
function get_running_round() {
  global $conn;
  $config = array('use_master_sched' => 0);  // May be overwritten

  $rs = odbc_exec($conn, 'select ItemKey, ItemValue from RaceInfo');

  while (odbc_fetch_row($rs)) {
    $key = odbc_result($rs, 'ItemKey');
    $val = odbc_result($rs, 'ItemValue');
    if ($key == 'ClassID')
      $config['classid'] = $val;
    else if ($key == 'RoundID')
      $config['roundid'] = $val;
    else if ($key == 'Heat')
      $config['heat'] = $val;
    else if ($key == 'use-master-sched')
      $config['use_master_sched'] = $val ? 1 : 0;
  }

  $rs = odbc_exec($conn, 'select Round, Class'
		  .' from Rounds'
		  .' inner join Classes'
		  .' on Rounds.ClassID = Classes.ClassID'
		  .' where RoundID = '.$config['roundid']
		  .' and Rounds.ClassID = '.$config['classid']);
  if (odbc_fetch_row($rs)) {
    $config['class'] = odbc_result($rs, 'Class');
    $config['round'] = odbc_result($rs, 'Round');
  }

  return $config;
}

// This HTML text will also get dynamically updated via update.js,
// so make sure any changes get made in both places!
function running_round_header($now_running, $groups_use_roundid = FALSE) {
  if ($now_running['class']) {
    echo '<h2 class="now_running">Now racing: '
      .'<a href="#group_'.$now_running[$groups_use_roundid ? 'roundid' : 'groupid'].'">'
      .($now_running['use_master_sched'] ? '' : $now_running['class'].', ')
      .'Round '.$now_running['round']
      .'</a>'
      .'</h2>'."\n";
  } else {
    echo '<h2 class="now_running">Racing has not yet begun.</h2>'."\n";
  }
}
?>