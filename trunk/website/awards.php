<?php @session_start();
// Summarizes all the awards (top speed awards, judged awards), by racer; makes it easier to present the awards
require_once('inc/data.inc');
require_once('inc/authorize.inc');
require_permission(VIEW_AWARDS_PERMISSION);
?><html>
<head>
<meta http-equiv="Content-Type" content="text/html; charset=UTF-8"/>
<title>Awards and Standings</title>
<?php require('inc/stylesheet.inc'); ?>
</head>
<body>
<?php $banner_title = 'Awards Summary'; require('inc/banner.inc');

require_once('inc/speed_trophies.inc');
require_once('inc/ordinals.inc');

$use_subgroups = read_raceinfo_boolean('use-subgroups');
$n_den_trophies = read_raceinfo('n-den-trophies', 3);
$n_pack_trophies = read_raceinfo('n-pack-trophies', 3);

$awards = array();  // RacerID -> award

// Collect the top N racer IDs in each class
$speed_trophies = top_finishers_by_class($n_den_trophies);
foreach ($speed_trophies as $classid => $racers) {
  for ($place = 0; $place < count($racers); ++$place) {
	$racerid = $racers[$place];
    $awards[$racerid] = ordinal(1 + $place).' in '.group_label_lc();
  }
}

$pack_trophies = top_finishers_overall($n_pack_trophies);
for ($place = 0; $place < count($pack_trophies); ++$place) {
  $racerid = $pack_trophies[$place];
  $awards[$racerid] .= ', '.ordinal(1 + $place).' in '.supergroup_label_lc();
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
  .'<th>'.htmlspecialchars(group_label(), ENT_QUOTES, 'UTF-8').'</th>'
  .($use_subgroups ? '<th>'.htmlspecialchars(subgroup_label(), ENT_QUOTES, 'UTF-8').'</th>' : '')
  .'<th>Last Name</th>'
  .'<th>First Name</th>'
  .'<th>Award(s)</th>'
  .'</tr>';

$sql = 'SELECT class, rank, carnumber, lastname, firstname, racerid'
    .' FROM '.inner_join('RegistrationInfo', 'Classes',
                         'RegistrationInfo.classid = Classes.classid',
                         'Ranks',
                         'RegistrationInfo.rankid = Ranks.rankid')
      .' WHERE passedinspection = 1 AND exclude = 0'
      .' ORDER BY carnumber';

foreach ($db->query($sql) as $row) {
  $racerid = $row['racerid'];
  echo '<tr>'
    .'<td>'.htmlspecialchars($row['carnumber'], ENT_QUOTES, 'UTF-8').'</td>'
    .'<td>'.htmlspecialchars($row['class'], ENT_QUOTES, 'UTF-8').'</td>'
    .($use_subgroups ? '<td>'.htmlspecialchars($row['rank'], ENT_QUOTES, 'UTF-8').'</td>' : '')
    .'<td>'.htmlspecialchars($row['lastname'], ENT_QUOTES, 'UTF-8').'</td>'
    .'<td>'.htmlspecialchars($row['firstname'], ENT_QUOTES, 'UTF-8').'</td>'
    .'<td>'.htmlspecialchars(@$awards[$racerid], ENT_QUOTES, 'UTF-8').'</td>'
    .'</tr>';
}
?>
</table>
</body>
</html>
