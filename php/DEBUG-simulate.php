<?php
require_once('data.inc');
date_default_timezone_set('America/New_York');

function set_current_heat($entry) {
  global $db;
  $db->exec("UPDATE RaceInfo SET itemvalue = '".$entry['classid']."' WHERE itemkey = 'ClassID'");
  $db->exec("UPDATE RaceInfo SET itemvalue = '".$entry['roundid']."' WHERE itemkey = 'RoundID'");
  $db->exec("UPDATE RaceInfo SET itemvalue = '".$entry['heat']."' WHERE itemkey = 'Heat'");
}

function racechart_where_condition($entry) {
	return " classid = ".$entry['classid']
	  ." AND roundid = ".$entry['roundid']
	  ." AND heat = ".$entry['heat'];
}

// Classes: ClassID, Class (name)
// Rounds:  RoundID, Round (int w/in class), ClassID, ChartType, Phase


// Compute an array of entries, each of which identifies one heat to simulate
function construct_race_order() {
  global $db;
    $sql = 'SELECT DISTINCT Classes.class, Classes.classid, round, Rounds.roundid, heat'
       .' FROM Rounds'
       .' INNER JOIN (Classes'
       .' INNER JOIN RaceChart'
       .' ON RaceChart.classid = Classes.classid)'
       .' ON RaceChart.roundid = Rounds.roundid'
       .' ORDER BY class, round, heat';
	$stmt = $db->query($sql);

	$race_order = array();
	foreach ($stmt as $rs) {
		$race_order[] = array("classid" => $rs['classid'],
							  "roundid" => $rs['roundid'],
							  "heat" => $rs['heat'],
							  "class" => $rs['class'],
							  "round" => $rs['round']
			);
	}

	return $race_order;
}

function backup_race_data() {
	@$db->exec('DROP TABLE Simulate_Rounds');
/*
	$db->exec('CREATE TABLE Simulate_Rounds ('
			  .' RoundID INTEGER NOT NULL AUTO_INCREMENT, '
			  .'  Round INTEGER NOT NULL, '
			  .'  ClassID INTEGER NOT NULL, '
			  .'  ChartType INTEGER, '
			  .'  Phase INTEGER NOT NULL, '
			  .'  INDEX (ClassID), '
			  .'  PRIMARY KEY (RoundID), '
			  .'  INDEX (Round), '
			  .'  INDEX (RoundID)'
			  .')');
	$db->exec('INSERT INTO Simulate_Rounds SELECT * FROM Rounds');
*/
	@$db->exec('DROP TABLE Simulate_RaceChart');
	$db->exec('CREATE TABLE Simulate_RaceChart ('
			  .'  ResultID INTEGER NOT NULL AUTO_INCREMENT, '
			  .'  ClassID INTEGER NOT NULL, '
			  .'  RoundID INTEGER NOT NULL, '
			  .'  Heat INTEGER NOT NULL, '
			  .'  Lane INTEGER NOT NULL, '
			  .'  RacerID INTEGER, '
			  .'  ChartNumber INTEGER, '
			  .'  FinishTime DOUBLE NULL, '
			  .'  FinishPlace INTEGER, '
			  .'  Points INTEGER, '
			  .'  Completed TIMESTAMP NULL, '
			  .'  IgnoreTime TINYINT(1) DEFAULT 0, '
			  .'  MasterHeat INTEGER DEFAULT 0, '
			  .'  INDEX (ChartNumber), '
			  .'  INDEX (ClassID), '
			  .'  INDEX (FinishTime), '
			  .'  INDEX (Heat), '
			  .'  INDEX (Lane), '
			  .'  INDEX (MasterHeat), '
			  .'  INDEX (Points), '
			  .'  PRIMARY KEY (ResultID), '
			  .'  INDEX (RacerID), '
			  .'  INDEX (RoundID)'
			  .')');
	$db->exec('INSERT INTO Simulate_RaceChart SELECT * FROM RaceChart');
}

backup_race_data();
echo "Backup completed\n";
$race_order = construct_race_order();

$db->exec('UPDATE Simulate_RaceChart SET completed = NULL');
$db->exec('DELETE FROM RaceChart');

$roundid = -1;
foreach ($race_order as $entry) {
	if ($entry['roundid'] != $roundid) {
		sleep(5);
		echo "Simulate scheduling ".$entry['class']." round ".$entry['round']
		     ." (round id ".$entry['roundid'].")...\n";
		$db->exec('INSERT INTO RaceChart'
				  .' SELECT * FROM Simulate_RaceChart'
				  .' WHERE roundid = '.$entry['roundid']);
		sleep(5);
		$roundid = $entry['roundid'];
	}
	set_current_heat($entry);
	echo 'ClassID '.$entry['classid'].' RoundID '.$entry['roundid'].' Heat '.$entry['heat']."\n";
	sleep(4);
	$now = date('Y-m-d H:i:s');
	$db->exec("UPDATE RaceChart rc"
			  ." SET finishtime = "
			  ." (SELECT finishtime FROM Simulate_RaceChart"
			  ."  WHERE ".racechart_where_condition($entry)
			  ."  AND lane = rc.lane"  // TODO: Don't know if this syntax works for Access
			  .")"
			  ." WHERE ".racechart_where_condition($entry));
	$db->exec("UPDATE RaceChart"
			  ." SET completed = '".$now."'"
			  ." WHERE ".racechart_where_condition($entry));
}

// Not sure what GPRM actually does here -- unset the ClassID/RoundID/Heat?  Advance to some nonexistent value?
set_current_heat(array('classid' => -1, 'roundid' => -1, 'heat' => -1));
?>