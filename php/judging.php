<?php @session_start(); ?>
<?php
require_once('data.inc');
require_once('authorize.inc');
require_permission(JUDGING_PERMISSION);
?>
<html>
<head>
<title>Award Judging</title>
<?php require('stylesheet.inc'); ?>
<meta http-equiv="refresh" content="300"/>
<script type="text/javascript" src="jquery.js"></script>
<script type="text/javascript" src="checkin.js"></script>
</head>
<body>
<?php
$banner_title = 'Judging';
require('banner.inc');

require_once('speed_trophies.inc');

$awardTypeOther = read_single_value('SELECT AwardTypeID FROM AwardTypes'
									.' WHERE AwardType = \'Other\'', 3);
$awards = Array(Array('Most', 'Aerodynamic'),
				Array('Most', 'Amazing'),
				Array('Best', 'Animal Theme'),
				Array('Most', 'Artistic'),
				Array('Most', 'Astonishing'),
				Array('Most', 'Attractive'),
				Array('Most', 'Awesome'),
				Array('Most', 'Box-Like'),
				Array('Most', 'Cheerful'),
				Array('Most', 'Colorful'),
				Array('', 'Coolest Car'),
				Array('Most', 'Cosmic'),
				Array('Most', 'Creative'),
				Array('Most', 'Creative Use of Materials'),
				Array('', 'Cubbiest Car'),
				Array('Most', 'Decals'),
				Array('Best', 'Detail'),
				Array('Most', 'Distinct'),
				Array('Most', 'Distinguished'),
				Array('Best', 'Driver in the Car'),
				Array('Most', 'Elaborate'),
				Array('Most', 'Expensive-Looking'),
				Array('Most', 'Extraordinary'),
				Array('Most', 'Extreme'),
				Array('', 'Fanciest'),
				Array('Most', 'Fascinating'),
				Array('', 'Fastest-Looking Car'),
				Array('Most', 'Funky'),
				Array('', 'Funniest'),
				Array('Most', 'Futuristic'),
				Array('Most', 'Galactic'),
				Array('', 'Glossiest'),
				Array('Most', 'Grandiose'),
				Array('Most', 'Humorous'),
				Array('Most', 'Imaginative Design'),
				Array('Most', 'Impressive'),
				Array('Most', 'Incredible'),
				Array('Most', 'Innovative'),
				Array('Most', 'Interesting'),
				Array('Most', 'Inventive'),
				Array('Most', 'Magnificent'),
				Array('Most', 'Majestic'),
				Array('Most', 'Original'),
				Array('Most', 'Outstanding'),
				Array('Most', 'Patriotic'),
				Array('Most', 'Phenomenal'),
				Array('Most', 'Radical'),
				Array('Most', 'Realistic-Looking'),
				Array('Most', 'Remarkable'),
				Array('Most', 'Resourceful'),
				Array('', 'Scariest'),
				Array('Best', 'Scout Theme'),
				Array('', 'Shiniest Car'),
				Array('', 'Sleekest Car'),
				Array('', 'Smoothest Finish'),
				Array('Most', 'Spirited'),
				Array('', 'Sportiest-Looking'),
				Array('Most', 'Stellar'),
				Array('', 'Strangest Shape'),
				Array('Most', 'Unusual'),
				Array('Best', 'Use of Color'),
				Array('Best', 'Use of the Color _____'),
				Array('Best', 'Vehicle Not a Car'),
				Array('Best', 'Wedge Shape'),
				Array('', 'Zaniest'));

$sql = 'SELECT RacerID, CarNumber, LastName, FirstName, Class'
      .' FROM RegistrationInfo'
      .' INNER JOIN Classes'
      .' ON Classes.ClassID = RegistrationInfo.ClassID'
      .' WHERE PassedInspection = 1 AND Exclude = 0'
      .' ORDER BY CarNumber';
$racers = Array();
$rs = odbc_exec($conn, $sql);
while (odbc_fetch_row($rs)) {
  $racerid = odbc_result($rs, 'RacerID');
  $racers[$racerid] = Array('RacerID' => $racerid,
							'CarNumber' => odbc_result($rs, 'CarNumber'),
							'LastName' => odbc_result($rs, 'LastName'),
							'FirstName' => odbc_result($rs, 'FirstName'),
							'Class' => odbc_result($rs, 'Class'),
							'Awards' => Array());
}

$n_den_trophies = read_raceinfo('n-den-trophies', 3);
$n_pack_trophies = read_raceinfo('n-pack-trophies', 3);
$ordinal = array(1 => '1st', 2 => '2nd', 3 => '3rd');
for ($i = 4; $i < 20; ++$i) $ordinal[$i] = $i.'th';

$speed_trophies = top_finishers_by_class($n_den_trophies);
foreach ($speed_trophies as $classid => $den_trophies) {
  for ($place = 0; $place < count($den_trophies); ++$place) {
	$racerid = $den_trophies[$place];
	$racers[$racerid]['Awards'][] = $ordinal[1 + $place].' in '.group_label_lc();
  }
}

$pack_trophies = top_finishers_overall($n_pack_trophies);
for ($place = 0; $place < count($pack_trophies); ++$place) {
  $racerid = $pack_trophies[$place];
  $racers[$racerid]['Awards'][] = $ordinal[1 + $place].' in '.supergroup_label_lc();
}

$sql = 'SELECT AwardName, RacerID'
  .' FROM Awards'
  .' ORDER BY Sort';


$rs = odbc_exec($conn, $sql);

while (odbc_fetch_row($rs)) {
  $racerid = odbc_result($rs, 'RacerID');
  if (isset($racers[$racerid]))
	$racers[$racerid]['Awards'][] = odbc_result($rs, 'AwardName');
}


  // Awards table:
  // AwardID, AwardName, AwardTypeID, ClassID, RankID, RacerID, Sort
  // 
  // AwardTypeID just maps to a string, AwardType, through AwardTypes string.

?>
<div class="award_racers">
<ul>
<?php
foreach ($racers as $racer) {
  echo '<li>'.$racer['CarNumber'].' - '.$racer['LastName'].', '.$racer['FirstName'];
  foreach ($racer['Awards'] as $award) {
	echo '; '.$award;
  }
  echo '</li>'."\n";
}
?>
</ul>
</div>

<div class="award_choices">
<table class="main_table">
<?php

$r = 0;
foreach ($awards as $award) {
  echo '<tr class="d'.($r & 1).'">';
  ++$r;
  echo '<td class="award_superlative">'.$award[0].'</td><td class="award_primary">'.$award[1].'</td>';
  echo '<td>';
  echo '<div class="block_buttons">';
  echo '<select>'."\n";
  echo '<option value="0">&lt;None&gt;</option>';
  foreach ($racers as $racer) {
	echo '<option value="'.$racer['RacerID'].'">'
	  .$racer['CarNumber'].' - '.$racer['LastName'].', '.$racer['FirstName']
	  .'</option>'."\n";
  }
  echo '</select>';
  echo '</div>';
  echo '</td>';
  echo '</tr>'."\n";
}
?>
</table>
</div>

</body>
</html>

  