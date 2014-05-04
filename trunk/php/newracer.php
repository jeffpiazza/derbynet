<?php @session_start(); 

// TODO: This page definitely needs some cosmetic help...

require_once('data.inc');
require_once('authorize.inc');
require_permission(REGISTER_NEW_RACER_PERMISSION);

if ($_POST) {
  if ($_POST['firstname'] and $_POST['lastname'] and $_POST['carno'] and $_POST['den']) {
    odbc_exec($conn, 'INSERT INTO RegistrationInfo (CarNumber, LastName, FirstName, RankID, ClassID, Exclude)'
	      .' VALUES('.$_POST['carno'].', \''.$_POST['lastname'].'\', \''.$_POST['firstname'].'\', '
	      .$_POST['den'] // $_POST['den'] is <rankid>,<classid>
	      .', '.($_POST['exclude'] ? 1 : 0).')');

    // The new racer won't be recognized without a Roster record to go with it.
    // Rounds has ChartType and Phase fields whose meaning isn't obvious.  This just enrolls
    // everyone into Round 1 for their Class.
    odbc_exec($conn, 'INSERT INTO Roster(RoundID, ClassID, RacerID)'
	      .' SELECT RoundID, RegistrationInfo.ClassID, RacerID'
	      .' FROM Rounds'
	      .' INNER JOIN RegistrationInfo'
	      .' ON Rounds.ClassID = RegistrationInfo.ClassID'
	      .' WHERE Round = 1'
	      .' AND NOT EXISTS(SELECT 1 FROM Roster'
	          .' WHERE Roster.RoundID = Rounds.RoundID'
	          .' AND Roster.ClassID = RegistrationInfo.ClassID'
	          .' AND Roster.RacerID = RegistrationInfo.RacerID)');

    odbc_close($conn);

    header('Location: checkin.php');
    exit();
  }
}
?>
<html>
<head>
<title>New Racer</title>
<?php require('stylesheet.inc'); ?>
</head>
<body>
<div class="block_buttons">
<form method="POST">
<span class="nrlabel">First name</span><span class="nrfield"><input type="text" name="firstname"/></span><br/>
<span class="nrlabel">Last name</span><span class="nrfield"><input type="text" name="lastname"/></span><br/>
<span class="nrlabel">Car number</span><span class="nrfield"><input type="text" name="carno"/></span><br/>
<span class="nrlabel">Den</span>
<span class="nrfield">
<select name="den">
<?php

$rs = odbc_exec($conn, 'SELECT max(Class & \'/\' & Rank) as MaxRank'
                       .' FROM Ranks'
                       .' INNER JOIN Classes'
                       .' ON Ranks.ClassID = Classes.ClassID');
odbc_fetch_row($rs);
$max_rank = odbc_result($rs, 'MaxRank');

$rs = odbc_exec($conn, 'SELECT RankID, Ranks.ClassID, Rank, Class'
                       .' FROM Ranks'
                       .' INNER JOIN Classes'
                       .' ON Ranks.ClassID = Classes.ClassID'
                       .' ORDER BY Class, Rank');
while (odbc_fetch_row($rs)) {
  echo '<option value="'.odbc_result($rs, 'RankID').','.odbc_result($rs, 'ClassID').'"'
    .(odbc_result($rs, 'Class').'/'.odbc_result($rs, 'Rank') == $max_rank ? ' selected="selected"' : '')
         .'>'.odbc_result($rs, 'Class').', '.odbc_result($rs, 'Rank').'</option>'."\n";
}

odbc_close($conn);

?>
</select>
</span><br>
<span class="nrlabel">Exclude?</span>
    <span class="nrfield"><input type="checkbox" name="exclude" value="exclude"/></span><br/>
<span class="nrlabel"> </span>
<span class="nrfield">
    <input type="submit" value="Submit"/>
    <input type="button" name="Cancel" value="Cancel" onclick="window.location = 'checkin.php';"/>
</span>
</form>
</div>
</body>
</html>