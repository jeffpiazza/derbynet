<?php @session_start();
require_once('inc/data.inc');
require_once('inc/authorize.inc');
require_permission(VIEW_AWARDS_PERMISSION);
?><html>
<head>
<title>Awards and Standings</title>
<?php require('inc/stylesheet.inc'); ?>
</head>
<body>
<?php
$banner_title = 'Awards Summary';
require('inc/banner.inc');

require_once('inc/data.inc');
require_once('inc/speed_trophies.inc');

$nlanes = get_lane_count();
$use_subgroups = read_raceinfo_boolean('use-subgroups');
$n_den_trophies = read_raceinfo('n-den-trophies', 3);
$n_pack_trophies = read_raceinfo('n-pack-trophies', 3);

$awards = array();  // RacerID -> award
$ordinal = array(1 => '1st', 2 => '2nd', 3 => '3rd');
for ($i = 4; $i < 20; ++$i) $ordinal[$i] = $i.'th';
// Beware 21st, etc.

// Collect the top N racer IDs in each class
$speed_trophies = top_finishers_by_class($n_den_trophies);
foreach ($speed_trophies as $classid => $racers) {
  for ($place = 0; $place < count($racers); ++$place) {
	$racerid = $racers[$place];
    $awards[$racerid] = $ordinal[1 + $place].' in '.group_label_lc();
  }
}

$pack_trophies = top_finishers_overall($n_pack_trophies);
for ($place = 0; $place < count($pack_trophies); ++$place) {
  $racerid = $pack_trophies[$place];
  $awards[$racerid] .= ', '.$ordinal[1 + $place].' in '.supergroup_label_lc();
}

// TODO: Order by den, then by award "weight" within each den.

foreach ($db->query('SELECT awardname, racerid'
					.' FROM Awards'
					.' ORDER BY sort') as $row) {
  $racerid = $row['racerid'];
  $comma = isset($awards[$racerid]) ? ', ' : '';
  @$awards[$racerid] .= $comma.$row['awardname'];
}

?>

<table>
<?php

echo '<tr>'
  .'<th>Car Number</th>'
  .'<th>'.group_label().'</th>'
  .($use_subgroups ? '<th>'.subgroup_label().'</th>' : '')
  .'<th>Last Name</th>'
  .'<th>First Name</th>'
  .'<th>Award(s)</th>'
  .'</tr>';

foreach ($db->query('SELECT class, rank, carnumber, lastname, firstname, racerid'
					.' FROM Ranks'
					.' INNER JOIN (Classes'
					.' INNER JOIN RegistrationInfo'
					.' ON RegistrationInfo.classid = Classes.classid)'
					.' ON RegistrationInfo.rankid = Ranks.rankid'
					.' WHERE passedinspection = 1 AND exclude = 0'
					.' ORDER BY carnumber') as $row) {
  $racerid = $row['racerid'];
  echo '<tr>'
    .'<td>'.$row['carnumber'].'</td>'
    .'<td>'.$row['class'].'</td>'
    .($use_subgroups ? '<td>'.$row['rank'].'</td>' : '')
    .'<td>'.$row['lastname'].'</td>'
    .'<td>'.$row['firstname'].'</td>'
    .'<td>'.@$awards[$racerid].'</td>'
    .'</tr>';
}
?>
</table>
</body>
</html>
