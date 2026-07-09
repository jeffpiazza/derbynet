<?php
// $_GET['awardid'] - The award ID to get eligible racers for

require_once('inc/data.inc');
require_once('inc/awards.inc');
require_once('inc/photo-config.inc');
require_once('inc/schema_version.inc');

$awardid = isset($_GET['awardid']) ? (int)$_GET['awardid'] : 0;

error_log("Fetching eligible racers for award ID: " . $awardid);

// Check if this is a design award - explicitly cast to integer for safety
$is_design = (int) read_single_value('SELECT COUNT(*) FROM AwardTypes 
                               INNER JOIN Awards ON AwardTypes.awardtypeid = Awards.awardtypeid 
                               WHERE Awards.awardid = :awardid 
                               AND (AwardTypes.awardtype = "Design General" OR AwardTypes.awardtype = "Design Trophy")',
                              array(':awardid' => $awardid));

error_log("Is design award check result: " . $is_design);

// Get award information (classid, rankid)
$award = read_single_row('SELECT classid, rankid FROM Awards WHERE awardid = :awardid',
                        array(':awardid' => $awardid), PDO::FETCH_ASSOC);

if ($is_design > 0) {
  // Fix the SQL query for carphoto
  $carphoto_field = (schema_version() < 2) ? '\'\' as carphoto' : 'r.carphoto';
  
  // For design awards, only show racers who have entered this category
  $sql = "SELECT r.racerid, r.firstname, r.lastname, r.carnumber, r.carname, 
          r.classid, r.rankid, r.imagefile, $carphoto_field
          FROM RegistrationInfo r
          INNER JOIN DesignEntries de ON r.racerid = de.racerid
          WHERE de.awardid = :awardid AND r.exclude = 0 AND r.passedinspection = 1
          ORDER BY r.lastname, r.firstname";
  
  error_log("Design award SQL: " . $sql);
  
  $stmt = $db->prepare($sql);
  $stmt->execute(array(':awardid' => $awardid));
  
  // Count results for debugging
  $racers_count = 0;
} else {
  // For non-design awards, show racers based on class/rank eligibility
  $sql = 'SELECT racerid, firstname, lastname, carnumber, carname, classid, rankid, 
          imagefile, ' . (schema_version() < 2 ? '\'\' as ' : '') . ' carphoto
          FROM RegistrationInfo 
          WHERE exclude = 0 AND passedinspection = 1';
  
  // Add class/rank filters if specified for the award
  if ($award && $award['classid'] > 0) {
    $sql .= ' AND classid = ' . $award['classid'];
  }
  if ($award && $award['rankid'] > 0) {
    $sql .= ' AND rankid = ' . $award['rankid'];
  }
  
  $sql .= ' ORDER BY lastname, firstname';
  $stmt = $db->query($sql);
}

$racers = array();
foreach ($stmt as $row) {
  $racers_count++;
  $carphoto_url = '';
  if (!empty($row['carphoto'])) {
    $carphoto_url = car_photo_repository()->lookup(RENDER_JUDGING)->render_url($row['carphoto']);
  }
  
  $racers[] = array(
    'racerid' => (int)$row['racerid'], // Ensure integer type
    'name' => $row['firstname'] . ' ' . $row['lastname'],
    'firstname' => $row['firstname'],
    'lastname' => $row['lastname'],
    'carnumber' => $row['carnumber'],
    'carname' => $row['carname'],
    'classid' => (int)$row['classid'],
    'rankid' => (int)$row['rankid'],
    'carphoto' => $carphoto_url
  );
}

error_log("Found $racers_count eligible racers for award ID: $awardid");

json_out('eligible_racers', $racers);
json_success();
?>
