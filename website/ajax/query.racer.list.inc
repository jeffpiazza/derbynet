<?php
//
// $_GET['order']:   name, class, rank, car, checkin
// $_GET['render']:  Name of the render to use for images


require_once('inc/schema_version.inc');
require_once('inc/events.inc');
require_once('inc/photo-config.inc');

$order = '';
if (isset($_GET['order']))
  $order = $_GET['order'];  // Values are: name, class, car
if (!$order)
    $order = 'name';

$render_name = RENDER_LISTVIEW;
if (isset($_GET['render']))
  $render_name = $_GET['render'];

$headshot_render = headshots()->lookup($render_name);
$carphoto_render = car_photo_repository()->lookup($render_name);

function order_by($order) {
  if ($order == 'car')
    return 'carnumber, lastname, firstname';
  if ($order == 'class')
    return 'class_sort, lastname, firstname';
  if ($order == 'rank')
    return 'class_sort, rank_sort, lastname, firstname';
  if ($order == 'checkin')
    return 'seq DESC';
  return 'lastname, firstname';
}

$schema_version = schema_version();

$sql = 'SELECT racerid, firstname, lastname, carnumber, carname, imagefile,'
    .($schema_version < 2 ? "'' AS carphoto" : " carphoto,")
    .($schema_version < 2 ? "class" : "Classes.sortorder").' AS class_sort,'
    .($schema_version < 2 ? "rank" : "Ranks.sortorder").' AS rank_sort,'
    .' RegistrationInfo.classid, class, RegistrationInfo.rankid, rank, passedinspection, exclude,'
    .' (SELECT MAX(seq) FROM Events'
    .'  WHERE Events.racerid = RegistrationInfo.racerid'
    .'    AND Events.action = '.EVENT_CHECKIN.') AS seq'
    .' FROM '.inner_join('RegistrationInfo', 'Classes',
                         'RegistrationInfo.classid = Classes.classid',
                         'Ranks',
                         'RegistrationInfo.rankid = Ranks.rankid')
    .' ORDER BY '.order_by($order);

$stmt = $db->query($sql);

echo "<racers>\n";
foreach ($stmt as $row) {
  echo '  <racer';
  echo ' racerid="'.$row['racerid'].'"';
  echo ' firstname="'.htmlspecialchars($row['firstname'], ENT_QUOTES, 'UTF-8').'"';
  echo ' lastname="'.htmlspecialchars($row['lastname'], ENT_QUOTES, 'UTF-8').'"';
  echo ' carnumber="'.$row['carnumber'].'"';
  echo ' carname="'.htmlspecialchars($row['carname'], ENT_QUOTES, 'UTF-8').'"';
  echo ' headshot="'.$headshot_render->render_url($row['imagefile']).'"';
  echo ' car_photo="'.$carphoto_render->render_url($row['imagefile']).'"';
  echo ' classid="'.$row['classid'].'"';
  echo ' class="'.htmlspecialchars($row['class'], ENT_QUOTES, 'UTF-8').'"';
  echo ' rankid="'.$row['rankid'].'"';
  echo ' rank="'.htmlspecialchars($row['rank'], ENT_QUOTES, 'UTF-8').'"';
  echo ' passed="'.$row['passedinspection'].'"';
  echo ' exclude="'.$row['exclude'].'"';
  echo ' seq="'.$row['seq'].'"';
  echo "/>\n";
}
echo "</racers>\n";
?>