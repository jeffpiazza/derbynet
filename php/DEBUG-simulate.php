<?php
require_once('data.inc');
date_default_timezone_set('America/New_York');

function set_current_heat($entry) {
	global $conn;
	odbc_exec($conn, "update RaceInfo set ItemValue = '".$entry['classid']."' where ItemKey = 'ClassID'");
	odbc_exec($conn, "update RaceInfo set ItemValue = '".$entry['roundid']."' where ItemKey = 'RoundID'");
	odbc_exec($conn, "update RaceInfo set ItemValue = '".$entry['heat']."' where ItemKey = 'Heat'");
}

function racechart_where_condition($entry) {
	return " ClassID = ".$entry['classid']
	  ." and RoundID = ".$entry['roundid']
	  ." and Heat = ".$entry['heat'];
}

// Classes: ClassID, Class (name)
// Rounds:  RoundID, Round (int w/in class), ClassID, ChartType, Phase


// Compute an array of entries, each of which identifies one heat to simulate
function construct_race_order() {
	global $conn;
    $sql = 'SELECT distinct classes.Class, classes.ClassID, Round, rounds.RoundID, Heat'
       .' from rounds'
       .' INNER JOIN (classes'
       .' INNER JOIN racechart'
       .' ON racechart.ClassID = classes.ClassID)'
       .' ON racechart.RoundID = rounds.RoundID'
       .' ORDER BY Class, Round, Heat';
	$rs = odbc_exec($conn, $sql);

	$race_order = array();
	while (odbc_fetch_row($rs)) {
		$race_order[] = array("classid" => odbc_result($rs, 'ClassID'),
							  "roundid" => odbc_result($rs, 'RoundID'),
							  "heat" => odbc_result($rs, 'Heat'),
							  "class" => odbc_result($rs, 'Class'),
							  "round" => odbc_result($rs, 'Round')
			);
	}

	return $race_order;
}

function backup_race_data() {
	global $conn;

	@odbc_exec($conn, 'DROP TABLE Simulate_Rounds');
/*
	odbc_exec($conn, 'CREATE TABLE Simulate_Rounds ('
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
	odbc_exec($conn, 'insert into Simulate_Rounds select * from Rounds');
*/
	@odbc_exec($conn, 'DROP TABLE Simulate_RaceChart');
	odbc_exec($conn, 'CREATE TABLE Simulate_RaceChart ('
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
	odbc_exec($conn, 'insert into Simulate_RaceChart select * from RaceChart');
}

backup_race_data();
echo "Backup completed\n";
$race_order = construct_race_order();

odbc_exec($conn, 'update Simulate_RaceChart set Completed = null');
odbc_exec($conn, 'delete from RaceChart');

$roundid = -1;
foreach ($race_order as $entry) {
	if ($entry['roundid'] != $roundid) {
		sleep(5);
		echo "Simulate scheduling ".$entry['class']." round ".$entry['round']
		     ." (round id ".$entry['roundid'].")...\n";
		odbc_exec($conn, 
				  'insert into RaceChart'
				  .' select * from Simulate_RaceChart'
				  .' where RoundID = '.$entry['roundid']);
		sleep(5);
		$roundid = $entry['roundid'];
	}
	set_current_heat($entry);
	echo 'ClassID '.$entry['classid'].' RoundID '.$entry['roundid'].' Heat '.$entry['heat']."\n";
	sleep(4);
	$now = date('Y-m-d H:i:s');
	odbc_exec($conn, "update RaceChart rc"
			  ." set FinishTime = "
			  ." (select FinishTime from Simulate_RaceChart"
			  ."  where ".racechart_where_condition($entry)
			  ."  and Lane = rc.Lane"  // TODO: Don't know if this syntax works for Access
			  .")"
			  ." where ".racechart_where_condition($entry));
	odbc_exec($conn, "update RaceChart"
			  ." set Completed = '".$now."'"
			  ." where ".racechart_where_condition($entry));
}

// Not sure what GPRM actually does here -- unset the ClassID/RoundID/Heat?  Advance to some nonexistent value?
set_current_heat(array('classid' => -1, 'roundid' => -1, 'heat' => -1));

odbc_close($conn);
?>