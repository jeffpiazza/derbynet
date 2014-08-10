<?php @session_start();

// EXPERIMENTAL
// Lists common non-specific awards (e.g., "Most Aerodynamic"), along
// with a drop-down selection element of potential recipients for
// each.
// TODO: Assigning an award presently has no effect!!
// TODO: Drag-and-drop, a la photo-thumbs
// TODO: Write-in field for true ad hoc awards

require_once('inc/data.inc');
require_once('inc/authorize.inc');
require_permission(JUDGING_PERMISSION);
?>
<html>
<head>
<meta http-equiv="Content-Type" content="text/html; charset=UTF-8"/>
<title>Award Judging</title>
<?php require('inc/stylesheet.inc'); ?>
<meta http-equiv="refresh" content="300"/>
<script type="text/javascript" src="js/jquery.js"></script>
<script type="text/javascript" src="js/checkin.js"></script>
</head>
<body>
<?php
$banner_title = 'Judging';
require('inc/banner.inc');

require_once('inc/speed_trophies.inc');

// TODO: Magical '3' for 'other' awards...
$awardTypeOther = read_single_value('SELECT awardtypeid FROM AwardTypes WHERE awardtype = \'Other\'',
									array(), 3);
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
				Array('Best', 'Use of the Color Blue'),
				Array('Best', 'Use of the Color Green'),
				Array('Best', 'Use of the Color Orange'),
				Array('Best', 'Use of the Color Purple'),
				Array('Best', 'Use of the Color Red'),
				Array('Best', 'Use of the Color Yellow'),
				Array('Best', 'Use of the Color _____'),
				Array('Best', 'Vehicle Not a Car'),
				Array('Best', 'Wedge Shape'),
				Array('', 'Zaniest'));

$sql = 'SELECT racerid, carnumber, lastname, firstname, class'
      .' FROM RegistrationInfo'
      .' INNER JOIN Classes'
      .' ON Classes.classid = RegistrationInfo.classid'
      .' WHERE passedinspection = 1 AND exclude = 0'
      .' ORDER BY carnumber';
$racers = array();
$stmt = $db->query($sql);
foreach ($stmt as $rs) {
  $racerid = $rs['racerid'];
  $racers[$racerid] = array('racerid' => $racerid,
							'carnumber' => $rs['carnumber'],
							'lastname' => $rs['lastname'],
							'firstname' => $rs['firstname'],
							'class' => $rs['class'],
							'awards' => array());
}
$stmt->closeCursor();

$n_den_trophies = read_raceinfo('n-den-trophies', 3);
$n_pack_trophies = read_raceinfo('n-pack-trophies', 3);
$ordinal = array(1 => '1st', 2 => '2nd', 3 => '3rd');
for ($i = 4; $i < 20; ++$i) $ordinal[$i] = $i.'th';

$speed_trophies = top_finishers_by_class($n_den_trophies);
foreach ($speed_trophies as $classid => $den_trophies) {
  for ($place = 0; $place < count($den_trophies); ++$place) {
	$racerid = $den_trophies[$place];
	$racers[$racerid]['awards'][] = $ordinal[1 + $place].' in '.group_label_lc();
  }
}

$pack_trophies = top_finishers_overall($n_pack_trophies);
for ($place = 0; $place < count($pack_trophies); ++$place) {
  $racerid = $pack_trophies[$place];
  $racers[$racerid]['awards'][] = $ordinal[1 + $place].' in '.supergroup_label_lc();
}

$sql = 'SELECT awardname, racerid'
  .' FROM Awards'
  .' ORDER BY sort';
$stmt = $db->query($sql);
foreach ($stmt as $rs) {
  $racerid = $rs['racerid'];
  if (isset($racers[$racerid]))
	$racers[$racerid]['awards'][] = $rs['awardname'];
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
  echo '<li>'.$racer['carnumber'].' - '.$racer['lastname'].', '.$racer['firstname'];
  foreach ($racer['awards'] as $award) {
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
	echo '<option value="'.$racer['racerid'].'">'
	  .$racer['carnumber'].' - '.$racer['lastname'].', '.$racer['firstname']
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
