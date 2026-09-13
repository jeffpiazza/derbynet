<?php
require_once('inc/awards.inc');

// Query to get all design awards (Design General and Design Trophy types)
$stmt = $db->query('SELECT a.awardid, a.awardname, a.classid, a.rankid, at.awardtype 
                   FROM Awards a
                   INNER JOIN AwardTypes at ON a.awardtypeid = at.awardtypeid
                   WHERE at.awardtype IN ("Design General", "Design Trophy")
                   ORDER BY a.sort');

$awards = array();
foreach ($stmt as $row) {
  $awards[] = array(
    'awardid' => (int)$row['awardid'], // Ensure integer type
    'awardname' => $row['awardname'],
    'classid' => (int)$row['classid'],
    'rankid' => (int)$row['rankid'],
    'awardtype' => $row['awardtype']
  );
}

// Log the results for debugging
error_log("Design awards query found " . count($awards) . " awards");
foreach ($awards as $award) {
  error_log("Design award ID: " . $award['awardid'] . ", Name: " . $award['awardname']);
}

json_out('awards', $awards);
json_success();
?>
