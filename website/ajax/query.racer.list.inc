<?php
//
// $_GET['order']:   name, class, rank, car, checkin
// $_GET['render']:  Name of the render to use for images

// $_GET['racerid']: If provided, restricts result to one racer
// $_GET['barcode']: If provided, restricts result to one racer

require_once('inc/barcode.inc');
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

$where = '';
$params = array();
if (@$_GET['racerid']) {
  $where = ' WHERE racerid = :racerid';
  $params[':racerid'] = $_GET['racerid'];
}
if (@$_GET['barcode']) {
  $where = ' WHERE racerid = :racerid';
  $params[':racerid'] = barcode_to_racerid($_GET['barcode']);
}

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
    .($schema_version < PARTITION_SCHEMA
      ? " 0 AS partitionid, class AS partition, '' AS note,"
      : " partitionid, (SELECT name FROM Partitions WHERE partitionid = RegistrationInfo.partitionid) AS partition, note,")
    .' RegistrationInfo.classid, class, RegistrationInfo.rankid, rank, passedinspection, exclude,'
     .($schema_version < REGISTRATION_CHECKIN_SCHEMA
       ? ' (SELECT MAX(seq) FROM Events'
       .'  WHERE Events.racerid = RegistrationInfo.racerid'
       .'    AND Events.action = '.EVENT_CHECKIN.') AS seq'
       : ' COALESCE(checkin_time, 0) AS seq')
    .' FROM '.inner_join('RegistrationInfo', 'Classes',
                         'RegistrationInfo.classid = Classes.classid',
                         'Ranks',
                         'RegistrationInfo.rankid = Ranks.rankid')
    .$where
    .' ORDER BY '.order_by($order);

$stmt = $db->prepare($sql);
$stmt->execute($params);

$racers = array();
foreach ($stmt as $row) {
  $racers[] = array('racerid' => $row['racerid'],
                    'firstname' => $row['firstname'],
                    'lastname' => $row['lastname'],
                    'carnumber' => $row['carnumber'],
                    'carname' => $row['carname'],
                    'partitionid' => $row['partitionid'],
                    'partition' => $row['partition'],
                    'note_from' => $row['note'],
                    'headshot' => $headshot_render->render_url($row['imagefile']),
                    'car_photo' => $carphoto_render->render_url($row['carphoto']),
                    'classid' => $row['classid'],
                    'class' => $row['class'],
                    'rankid' => $row['rankid'],
                    'rank' => $row['rank'],
                    'passed' => $row['passedinspection'] ? true : false,
                    'seq' => $row['seq']);
}
json_out('racers', $racers);

?>
