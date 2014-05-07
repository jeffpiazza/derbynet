<?php
/*

  Classes: ClassID, Class
  Ranks: RankID, Rank, ClassID
  Rounds: RoundID, Round, ClassID, ChartType, Phase
  RegistrationInfo: RacerID, CarNumber, CarName, FirstName, LastName, ClassID, RankID, PassedInspection, ImageFile, Exclude
  RaceChart: ResultID, ClassID, RoundID, Heat, Lane, RacerID, ChartNumber, FinishTime, FinishPlace, Points, Completed, 
             IgnoreTime, MasterHeat
  Roster: RosterID, RoundID, ClassID, RacerID, Finalist, GrandFinalist
 */
session_start(); ?>
<!DOCTYPE html>
<html>
<head>
<title>Database Table Dumps</title>
</head>
<body>
<h2>Database Table Dumps</h2>
<?php 

require_once('data.inc');


// This query doesn't work: unsupported join
if (false) {
  /*
  .' FROM classes'
  .' RIGHT JOIN (rounds'
  .' LEFT JOIN (roster'
  .' LEFT JOIN (registrationinfo'
  .' LEFT JOIN racechart'
  .' ON registrationinfo.RacerID = racechart.RacerID)'
  .' ON (roster.RacerID = registrationinfo.RacerID)'
  .' AND(roster.RoundID = racechart.RoundID))'
  .' ON rounds.RoundID = roster.RoundID)'
  .' ON rounds.ClassID = classes.ClassID'
  // If racer has any racechart entries (for other rounds), then 
  // this will filter out all unscheduled rounds for that racer
  //.' WHERE rounds.RoundID = racechart.RoundID'
  //.'  OR racechart.RoundID is null'
  */

$sql ='SELECT'
  .' classes.Class, Round, Heat, Lane, FinishTime, ResultID, Completed, '
  .' registrationinfo.CarNumber, registrationinfo.FirstName, registrationinfo.LastName,'
  .' classes.ClassID, rounds.RoundID, racechart.RacerID'
.' FROM (RegistrationInfo INNER JOIN ((Classes INNER JOIN Rounds ON Classes.ClassID = Rounds.ClassID) LEFT JOIN Roster ON Rounds.RoundID = Roster.RoundID) ON RegistrationInfo.RacerID = Roster.RacerID) LEFT JOIN RaceChart ON (Roster.RoundID = RaceChart.RoundID) AND (RegistrationInfo.RacerID = RaceChart.RacerID)'

  .' ORDER BY class, round, heat, lane';

?>

<table>
<tr><th>Class</th><th>Round</th><th>Heat</th><th>Lane</th>
    <th>ResultID</th><th>FirstName</th><th>Completed</th>
</tr>
<?php

$stmt = $db->query($sql);
if ($stmt === FALSE) {
  $info = $db->errorInfo();
  echo '<tr><td>Error: '.$info[2].'</td></tr>'."\n";
}

foreach ($stmt as $rs) {
  echo '<tr><td>'.$rs['class'].'</td>'
    .'<td>'.$rs['round'].' (ID='.$rs['roundid'].')</td>'
    .'<td>'.$rs['heat'].'</td>'
    .'<td>'.$rs['lane'].'</td>'
    .'<td>'.$rs['resultid'].'</td>'
    .'<td>'.$rs['firstname'].'</td>'
    .'<td>'.$rs['completed'].'</td>'
    .'</tr>'."\n";
 }
?>
</table>
<?php
	}
?>

<p>RaceInfo table</p>
<table>
<?php
foreach ($db->query('SELECT itemkey, itemvalue FROM RaceInfo') as $rs) {
  echo '<tr><td>'.$rs['itemkey'].'</td><td>'.$rs['itemvalue'].'</td></tr>'."\n";
}
?>
</table>

<p>Classes Table</p>
<table>
<tr><th>ClassID</th><th>Class</th></tr>
<?php
foreach ($db->query('SELECT classid, class FROM Classes ORDER BY classid') as $rs) {
	echo '<tr><td>'.$rs['classid'].'</td>'
	.'<td>'.$rs['class'].'</td></tr>'."\n";
}
?>
</table>

<p>Ranks Table</p>
<table>
<tr><th>RankID</th><th>Rank</th><th>ClassID</th></tr>
<?php
foreach ($db->query('SELECT rankid, rank, classid FROM Ranks ORDER BY rankid') as $rs) {
	echo '<tr><td>'.$rs['rankid'].'</td>'
	.'<td>'.$rs['rank'].'</td>'
	.'<td>'.$rs['classid'].'</td></tr>'."\n";
}
?>
</table>

<?php
  // This doesn't work, either:
if (false) {
?>
<p>Classes and Ranks</p>
<table>
<tr><th>ClassID</th><th>RankID</th><th>Rank</th><th>Registered</th><th>Passed</th></tr>
<?php
foreach ($db->query('SELECT Ranks.classid, Ranks.rankid, rank, COUNT(*) AS n, SUM(passedinspection) AS passed'
					.' FROM RegistrationInfo'
					.' JOIN Ranks'
					.' ON RegistrationInfo.rankid = Ranks.rankid'
					.' GROUP BY classid, Ranks.rankid ORDER BY classid, Ranks.rankid') as $rs) {
	echo '<tr><td>'.$rs['classid'].'</td>'
	.'<td>'.$rs['rankid'].'</td>'
	.'<td>'.$rs['rank'].'</td>'
	.'<td>'.$rs['n'].'</td>'
	.'<td>'.$rs['passed'].'</td>'
	.'</tr>'."\n";
}
?>
</table>
<?php
	}
?>
<p>Roster</p>
<table>
<tr><th>RosterID</th><th>RoundID</th><th>ClassID</th><th>RacerID</th><th>Finalist</th><th>Grand Finalist</th></tr>
<?php
foreach ($db->query('SELECT rosterid, roundid, classid, racerid, finalist, grandfinalist'
					.' FROM Roster ORDER BY rosterid') as $rs) {
	echo '<tr><td>'.$rs['rosterid'].'</td>'
	.'<td>'.$rs['roundid'].'</td>'
	.'<td>'.$rs['classid'].'</td>'
	.'<td>'.$rs['racerid'].'</td>'
	.'<td>'.$rs['finalist'].'</td>'
	.'<td>'.$rs['grandfinalist'].'</td>'
	.'</tr>'."\n";
}
?>
</table>


<p>Rounds Table</p>
<table>
<tr><th>Round ID</th><th>Round</th><th>ClassID</th><th>Class</th><th>ChartType</th><th>Phase</th>
    <th>In Roster</th>
    <th>Scheduled<br/>Heats</th><th>Completed<br/>Heats</th></tr>
<?php
  // Classes table: ClassID, Class
  // Rounds table: Round, RoundID, ClassID
  // RegistrationInfo table: CarNumber, FirstName, LastName
  // racechart table: ResultID, Lane, FinishTime, Completed (time)
$sql = 'SELECT roundid, round, Rounds.classid, class, charttype, phase'
       .' FROM Rounds'
       .' LEFT JOIN Classes'
       .' ON Rounds.classid = Classes.classid'
       .' ORDER BY roundid';
$stmt = $db->query($sql);
$rounds = array();
foreach ($stmt as $rs) {
	$rounds[] = array('roundid' => $rs['roundid'],
					  'round' => $rs['round'],
					  'classid' => $rs['classid'],
					  'class' => $rs['class'],
					  'charttype' => $rs['charttype'],
					  'phase' => $rs['phase']);
}

foreach ($rounds as $round) {
	echo '<tr><td>'.$round['roundid'].'</td>'
	.'<td>'.$round['round'].'</td>'
	.'<td>'.$round['classid'].'</td>'
	.'<td>'.$round['class'].'</td>'
	.'<td>'.$round['charttype'].'</td>'
	.'<td>'.$round['phase'].'</td>';
	echo '<td>'.read_single_value('SELECT COUNT(*) FROM Roster WHERE roundid = '.$round['roundid']).'</td>';
	echo '<td>'.read_single_value('SELECT COUNT(*) FROM RaceChart WHERE roundid = '.$round['roundid']).'</td>';
	echo '<td>'.read_single_value('SELECT COUNT(*) FROM RaceChart WHERE roundid = '.$round['roundid']
								  .' AND completed IS NOT NULL').'</td>';
	echo '</tr>'."\n";
}
?>
</table>

<p>RaceChart</p>
<table>
<tr><th>ResultID</th><th>ClassID</th><th>RoundID</th><th>Heat</th><th>Lane</th><th>RacerID</th>
    <th>FinishTime</th><th>Completed</th></tr>
<?php
foreach ($db->query('SELECT resultid, classid, roundid, heat, lane, racerid, finishtime, completed'
					.' FROM RaceChart ORDER BY resultid') as $rs) {
	echo '<tr>'
	.'<td>'.$rs['resultid'].'</td>'
	.'<td>'.$rs['classid'].'</td>'
	.'<td>'.$rs['roundid'].'</td>'
	.'<td>'.$rs['heat'].'</td>'
	.'<td>'.$rs['lane'].'</td>'
	.'<td>'.$rs['racerid'].'</td>'
	.'<td>'.$rs['finishtime'].'</td>'
	.'<td>'.$rs['completed'].'</td>'
	.'</tr>'."\n";
}
?>
</table>

</body>
</html>
