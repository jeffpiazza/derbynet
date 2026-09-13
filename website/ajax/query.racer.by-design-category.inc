<?php
// $_GET['awardid'] - Award ID to filter by

require_once('inc/data.inc');
require_once('inc/schema_version.inc');
require_once('inc/photo-config.inc');

$awardid = isset($_GET['awardid']) ? (int)$_GET['awardid'] : 0;

if ($awardid <= 0) {
  json_failure('noaward', 'No award ID specified');
  exit;
}

// Simpler query to avoid SQL errors
$sql = "SELECT r.racerid, r.carnumber, r.lastname, r.firstname, r.carname, 
        r.classid, r.rankid, r.imagefile, 
        " . (schema_version() < 2 ? "'' as carphoto" : "r.carphoto") . "
        FROM RegistrationInfo r
        INNER JOIN DesignEntries de ON r.racerid = de.racerid
        WHERE de.awardid = :awardid
        AND r.passedinspection = 1
        ORDER BY r.lastname, r.firstname";

$stmt = $db->prepare($sql);
$stmt->execute(array(':awardid' => $awardid));

$racers = array();
$rowno = 0;
foreach ($stmt as $rs) {
  $rowno++;
  $racers[] = array(
    'racerid' => $rs['racerid'],
    'carnumber' => $rs['carnumber'],
    'lastname' => $rs['lastname'],
    'firstname' => $rs['firstname'],
    'carname' => $rs['carname'],
    'classid' => $rs['classid'],
    'rankid' => $rs['rankid'],
    'headshot' => headshot_repository()->lookup(RENDER_TINY)->get_url_for_racer_icon($rs['racerid'], $rs['imagefile']),
    'rowno' => $rowno
  );
}

json_out('racers', $racers);
json_success();
?>
