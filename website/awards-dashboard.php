<?php @session_start();
// Controls the "current award" kiosk display
require_once('inc/data.inc');
require_once('inc/authorize.inc');
require_once('inc/schema_version.inc');
require_permission(PRESENT_AWARDS_PERMISSION);
?><html>
<head>
<meta http-equiv="Content-Type" content="text/html; charset=UTF-8"/>
<title>Awards Presentation Dashboard</title><?php require('inc/stylesheet.inc'); ?>
<script type="text/javascript" src="js/jquery.js"></script>
<script type="text/javascript" src="js/jquery-ui-1.10.4.min.js"></script>
<script type="text/javascript" src="js/dashboard-ajax.js"></script>
<script type="text/javascript" src="js/awards-dashboard.js"></script>
</head>
<body>
<?php $banner_title = 'Awards Presentation'; require('inc/banner.inc');

require_once('inc/standings.inc');
require_once('inc/ordinals.inc');

$use_subgroups = read_raceinfo_boolean('use-subgroups');
$n_den_trophies = read_raceinfo('n-den-trophies', 3);
$n_pack_trophies = read_raceinfo('n-pack-trophies', 3);

$awards = array();  // RacerID -> array of { awardname, key }

// Collect the top N racer IDs in each class
$speed_trophies = top_finishers_by_class($n_den_trophies);
foreach ($speed_trophies as $classid => $racers) {
  for ($place = 0; $place < count($racers); ++$place) {
	$racerid = $racers[$place];
    if (!isset($awards[$racerid])) {
      $awards[$racerid] = array();
    }
    $awards[$racerid][] = array('awardname' => ordinal(1 + $place).' in '.group_label_lc(),
                                'key' => 'speed-'.(1 + $place).'-'.$classid);
  }
}

$pack_trophies = top_finishers_overall($n_pack_trophies);
for ($place = 0; $place < count($pack_trophies); ++$place) {
  $racerid = $pack_trophies[$place];
  if (!isset($awards[$racerid])) {
    $awards[$racerid] = array();
  }
  $awards[$racerid][] = array('awardname' => ordinal(1 + $place).' in '.supergroup_label_lc(),
                              'key' => 'speed-'.(1 + $place));
}

// TODO: Order by den, then by award "weight" within each den.

foreach ($db->query('SELECT awardid, awardname, racerid'
					.' FROM Awards'
					.' ORDER BY sort') as $row) {
  $racerid = $row['racerid'];
  if (!isset($awards[$racerid])) {
    $awards[$racerid] = array();
  }
  $awards[$racerid][] = array('awardname' => $row['awardname'],
                              'key' => 'award-'.$row['awardid']);
}
?>

<div class="block_buttons">
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
      .' ORDER BY '
      .(schema_version() >= 2 ? 'Classes.sortorder, ' : '')
      .'class, lastname, firstname, carnumber';

foreach ($db->query($sql) as $racer) {
  $racerid = $racer['racerid'];
  if (!isset($awards[$racerid])) {
      echo '<tr>'
      .'<td>'.htmlspecialchars($racer['carnumber'], ENT_QUOTES, 'UTF-8').'</td>'
      .'<td>'.htmlspecialchars($racer['class'], ENT_QUOTES, 'UTF-8').'</td>'
      .($use_subgroups ? '<td>'.htmlspecialchars($racer['rank'], ENT_QUOTES, 'UTF-8').'</td>' : '')
      .'<td>'.htmlspecialchars($racer['lastname'], ENT_QUOTES, 'UTF-8').'</td>'
      .'<td>'.htmlspecialchars($racer['firstname'], ENT_QUOTES, 'UTF-8').'</td>'
      .'<td>'.htmlspecialchars('--', ENT_QUOTES, 'UTF-8').'</td>'
      .'</tr>';
  } else {
    foreach ($awards[$racerid] as $award) {
      echo '<tr>'
      .'<td>'.htmlspecialchars($racer['carnumber'], ENT_QUOTES, 'UTF-8').'</td>'
      .'<td>'.htmlspecialchars($racer['class'], ENT_QUOTES, 'UTF-8').'</td>'
      .($use_subgroups ? '<td>'.htmlspecialchars($racer['rank'], ENT_QUOTES, 'UTF-8').'</td>' : '')
      .'<td>'.htmlspecialchars($racer['lastname'], ENT_QUOTES, 'UTF-8').'</td>'
      .'<td>'.htmlspecialchars($racer['firstname'], ENT_QUOTES, 'UTF-8').'</td>'

      .'<td><input type="button" data-enhanced="true"'
          .' onclick="present_award(this)"'
          .' data-award="'.$award['key'].'"'
          .' value="'.htmlspecialchars($award['awardname'], ENT_QUOTES, 'UTF-8').'"/>'
      .'</td>'
      .'</tr>';
    }
  }
}
?>
</table>
</div>
</body>
</html>
