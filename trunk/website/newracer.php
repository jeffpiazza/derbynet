<?php @session_start(); 

// TODO: This page definitely needs some cosmetic help...

require_once('inc/data.inc');
require_once('inc/authorize.inc');
require_permission(REGISTER_NEW_RACER_PERMISSION);

if ($_POST) {
  if ($_POST['firstname'] and $_POST['lastname'] and $_POST['carno'] and $_POST['den']) {
	// TODO: PDO constructor shouldn't be here; just an interim solution
	  $stmt = $db->prepare('INSERT INTO RegistrationInfo (CarNumber, LastName, FirstName, RankID, ClassID, Exclude)'
						   .' VALUES(:carno, :lastname, :firstname, '
						   .$_POST['den'] // $_POST['den'] is <rankid>,<classid>
						   .', '.($_POST['exclude'] ? 1 : 0).')');
	  $stmt->execute(array(':carno' => $_POST['carno'],
						   ':firstname' => $_POST['firstname'],
						   ':lastname' => $_POST['lastname']));

	  // The new racer won't be recognized without a Roster record to go with it.
	  // Rounds has ChartType and Phase fields whose meaning isn't obvious.  This just enrolls
	  // everyone into Round 1 for their Class.
	  $db->exec('INSERT INTO Roster(RoundID, ClassID, RacerID)'
				.' SELECT RoundID, RegistrationInfo.ClassID, RacerID'
				.' FROM Rounds'
				.' INNER JOIN RegistrationInfo'
				.' ON Rounds.ClassID = RegistrationInfo.ClassID'
				.' WHERE Round = 1'
				.' AND NOT EXISTS(SELECT 1 FROM Roster'
				.' WHERE Roster.RoundID = Rounds.RoundID'
				.' AND Roster.ClassID = RegistrationInfo.ClassID'
				.' AND Roster.RacerID = RegistrationInfo.RacerID)');

    header('Location: checkin.php');
    exit();
  }
}
?>
<html>
<head>
<title>New Racer</title>
<?php require('inc/stylesheet.inc'); ?>
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

$max_rank = read_single_value('SELECT MAX(class & \'/\' & rank) AS maxrank'
							  .' FROM Ranks'
							  .' INNER JOIN Classes'
							  .' ON Ranks.classid = Classes.classid', array());
$stmt = $db->query('SELECT rankid, Ranks.classid, rank, class'
				   .' FROM Ranks'
				   .' INNER JOIN Classes'
				   .' ON Ranks.classid = Classes.classid'
				   .' ORDER BY class, rank');
foreach ($stmt as $rs) {
  echo '<option value="'.$rs['rankid'].','.$rs['classid'].'"'
    .($rs['class'].'/'.$rs['rank'] == $max_rank ? ' selected="selected"' : '')
	.'>'.$rs['class'].', '.$rs['rank'].'</option>'."\n";
}

?>
</select>
</span><br/>
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