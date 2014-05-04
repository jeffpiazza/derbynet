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

$sql ='SELECT'
  .' classes.Class, Round, Heat, Lane, FinishTime, ResultID, Completed, '
  .' registrationinfo.CarNumber, registrationinfo.FirstName, registrationinfo.LastName,'
  .' classes.ClassID, rounds.RoundID, racechart.RacerID'
.' FROM (RegistrationInfo INNER JOIN ((Classes INNER JOIN Rounds ON Classes.ClassID = Rounds.ClassID) LEFT JOIN Roster ON Rounds.RoundID = Roster.RoundID) ON RegistrationInfo.RacerID = Roster.RacerID) LEFT JOIN RaceChart ON (Roster.RoundID = RaceChart.RoundID) AND (RegistrationInfo.RacerID = RaceChart.RacerID)'
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

  .' ORDER BY Class, Round, Heat, Lane';


?>

<table>
<tr><th>Class</th><th>Round</th><th>Heat</th><th>Lane</th>
    <th>ResultID</th><th>FirstName</th><th>Completed</th>
</tr>
<?php
$rs = odbc_exec($conn, $sql);
if ($rs === FALSE) {
  echo '<tr><td>Error: '.odbc_errormsg($conn).'</td></tr>'."\n";
}

while (odbc_fetch_row($rs)) {
  echo '<tr><td>'.odbc_result($rs, 'Class').'</td>'
    .'<td>'.odbc_result($rs, 'Round').' (ID='.odbc_result($rs, 'RoundID').')</td>'
    .'<td>'.odbc_result($rs, 'Heat').'</td>'
    .'<td>'.odbc_result($rs, 'Lane').'</td>'
    .'<td>'.odbc_result($rs, 'ResultID').'</td>'
    .'<td>'.odbc_result($rs, 'FirstName').'</td>'
    .'<td>'.odbc_result($rs, 'Completed').'</td>'
    .'</tr>'."\n";
 }
?>
</table>

<p>RaceInfo table</p>
<table>
<?php
$rs = odbc_exec($conn, 'select ItemKey, ItemValue from RaceInfo');

while (odbc_fetch_row($rs)) {
  $key = odbc_result($rs, 'ItemKey');
  $val = odbc_result($rs, 'ItemValue');
  echo '<tr><td>'.$key.'</td><td>'.$val.'</td></tr>'."\n";
 }
?>
</table>

<p>Classes Table</p>
<table>
<tr><th>ClassID</th><th>Class</th></tr>
<?php
$rs = odbc_exec($conn, 'select ClassID, Class from Classes order by ClassID');
while (odbc_fetch_row($rs)) {
	echo '<tr><td>'.odbc_result($rs, 'ClassID').'</td>'
	.'<td>'.odbc_result($rs, 'Class').'</td></tr>'."\n";
}
?>
</table>

<p>Ranks Table</p>
<table>
<tr><th>RankID</th><th>Rank</th><th>ClassID</th></tr>
<?php
$rs = odbc_exec($conn, 'select RankID, Rank, ClassID from Ranks order by RankID');
while (odbc_fetch_row($rs)) {
	echo '<tr><td>'.odbc_result($rs, 'RankID').'</td>'
	.'<td>'.odbc_result($rs, 'Rank').'</td>'
	.'<td>'.odbc_result($rs, 'ClassID').'</td></tr>'."\n";
}
?>
</table>

<p>Classes and Ranks</p>
<table>
<tr><th>ClassID</th><th>RankID</th><th>Rank</th><th>Registered</th><th>Passed</th></tr>
<?php
$rs = odbc_exec($conn, 'select ranks.ClassID, ranks.RankID, Rank, count(*) as n, sum(PassedInspection) as passed'
				.' from registrationinfo'
				.' join ranks'
				.' on registrationinfo.RankID = ranks.RankID'
				.' group by ClassID, ranks.RankID order by ClassID, ranks.RankID');
while (odbc_fetch_row($rs)) {
	echo '<tr><td>'.odbc_result($rs, 'ClassID').'</td>'
	.'<td>'.odbc_result($rs, 'RankID').'</td>'
	.'<td>'.odbc_result($rs, 'Rank').'</td>'
	.'<td>'.odbc_result($rs, 'n').'</td>'
	.'<td>'.odbc_result($rs, 'passed').'</td>'
	.'</tr>'."\n";
}
?>
</table>

<p>Roster</p>
<table>
<tr><th>RosterID</th><th>RoundID</th><th>ClassID</th><th>RacerID</th><th>Finalist</th><th>Grand Finalist</th></tr>
<?php
$rs = odbc_exec($conn, 'select RosterID, RoundID, ClassID, RacerID, Finalist, GrandFinalist'
				.' from Roster order by RosterID');
while (odbc_fetch_row($rs)) {
	echo '<tr><td>'.odbc_result($rs, 'RosterID').'</td>'
	.'<td>'.odbc_result($rs, 'RoundID').'</td>'
	.'<td>'.odbc_result($rs, 'ClassID').'</td>'
	.'<td>'.odbc_result($rs, 'RacerID').'</td>'
	.'<td>'.odbc_result($rs, 'Finalist').'</td>'
	.'<td>'.odbc_result($rs, 'GrandFinalist').'</td>'
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
$sql = 'select RoundID, Round, rounds.ClassID, Class, ChartType, Phase'
       .' FROM rounds'
       .' LEFT JOIN classes'
       .' ON rounds.ClassID = classes.ClassID'
       .' ORDER BY RoundID';
$rs = odbc_exec($conn, $sql);
$rounds = array();
while (odbc_fetch_row($rs)) {
	$rounds[] = array('RoundID' => odbc_result($rs, 'RoundID'),
					 'Round' => odbc_result($rs, 'Round'),
					 'ClassID' => odbc_result($rs, 'ClassID'),
					 'Class' => odbc_result($rs, 'Class'),
					  'ChartType' => odbc_result($rs, 'ChartType'),
					  'Phase' => odbc_result($rs, 'Phase'));
}

foreach ($rounds as $round) {
	echo '<tr><td>'.$round['RoundID'].'</td>'
	.'<td>'.$round['Round'].'</td>'
	.'<td>'.$round['ClassID'].'</td>'
	.'<td>'.$round['Class'].'</td>'
	.'<td>'.$round['ChartType'].'</td>'
	.'<td>'.$round['Phase'].'</td>';
	$rs = odbc_exec($conn, 'select count(*) from roster where RoundID = '.$round['RoundID']);
	odbc_fetch_row($rs);
	echo '<td>'.odbc_result($rs, 1).'</td>';
	$rs = odbc_exec($conn, 'select count(*) from racechart where RoundID = '.$round['RoundID']);
	odbc_fetch_row($rs);
	echo '<td>'.odbc_result($rs, 1).'</td>';
	$rs = odbc_exec($conn, 'select count(*) from racechart where RoundID = '.$round['RoundID']
					.' AND Completed is not null');
	odbc_fetch_row($rs);
	echo '<td>'.odbc_result($rs, 1).'</td>';
	echo "\n";
}
?>
</table>

<p>RaceChart</p>
<table>
<tr><th>ResultID</th><th>ClassID</th><th>RoundID</th><th>Heat</th><th>Lane</th><th>RacerID</th>
    <th>FinishTime</th><th>Completed</th></tr>
<?php
$rs = odbc_exec($conn, 'select ResultID, ClassID, RoundID, Heat, Lane, RacerID, FinishTime, Completed'
				.' from RaceChart order by ResultID');
while (odbc_fetch_row($rs)) {
	echo '<tr>'
	.'<td>'.odbc_result($rs, 'ResultID').'</td>'
	.'<td>'.odbc_result($rs, 'ClassID').'</td>'
	.'<td>'.odbc_result($rs, 'RoundID').'</td>'
	.'<td>'.odbc_result($rs, 'Heat').'</td>'
	.'<td>'.odbc_result($rs, 'Lane').'</td>'
	.'<td>'.odbc_result($rs, 'RacerID').'</td>'
	.'<td>'.odbc_result($rs, 'FinishTime').'</td>'
	.'<td>'.odbc_result($rs, 'Completed').'</td>'
	.'</tr>'."\n";
}
?>
</table>

</body>
<?php
odbc_close($conn);
?>
</html>
