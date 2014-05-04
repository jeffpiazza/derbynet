<?php @session_start();
require_once('data.inc');
require_once('authorize.inc');
require_permission(VIEW_AWARDS_PERMISSION);
?><html>
<head>
<title>Awards and Standings</title>
<?php require('stylesheet.inc'); ?>
</head>
<body>
<?php
$banner_title = 'Awards Summary';
require('banner.inc');

require_once('data.inc');
require_once('speed_trophies.inc');

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

$sql = 'SELECT AwardName, RacerID'
  .' FROM Awards'
  .' ORDER BY Sort';


$rs = odbc_exec($conn, $sql);

while (odbc_fetch_row($rs)) {
  $racerid = odbc_result($rs, 'RacerID');
  $comma = isset($awards[$racerid]) ? ', ' : '';
  $awards[$racerid] .= $comma.odbc_result($rs, 'AwardName');
}

?>

<table>
<?php

$sql = 'SELECT Class, Rank, CarNumber, LastName, FirstName, RacerID'
  .' FROM Ranks'
  .' INNER JOIN (Classes'
  .' INNER JOIN RegistrationInfo'
  .' ON RegistrationInfo.ClassID = Classes.ClassID)'
  .' ON RegistrationInfo.RankID = Ranks.RankID'
  .' WHERE PassedInspection = 1 AND Exclude = 0'
  .' ORDER BY CarNumber';

$rs = odbc_exec($conn, $sql);

echo '<tr>'
  .'<th>Car Number</th>'
  .'<th>'.group_label().'</th>'
  .($use_subgroups ? '<th>'.subgroup_label().'</th>' : '')
  .'<th>Last Name</th>'
  .'<th>First Name</th>'
  .'<th>Award(s)</th>'
  .'</tr>';

while (odbc_fetch_row($rs)) {
  echo '<tr>'
    .'<td>'.odbc_result($rs, 'CarNumber').'</td>'
    .'<td>'.odbc_result($rs, 'Class')
    .($use_subgroups ? '<td>'.odbc_result($rs, 'Rank').'</td>' : '')
    .'</td>'
    .'<td>'.odbc_result($rs, 'LastName').'</td>'
    .'<td>'.odbc_result($rs, 'FirstName').'</td>'
    .'<td>'.$awards[odbc_result($rs, 'RacerID')].'</td>'
    .'</tr>';
}

odbc_close($conn);
?>
</table>
</body>
</html>
