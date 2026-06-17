<?php
// $_POST['awardid'] - The award ID to auto-enter all racers for
require_once('inc/permissions.inc');

if (!have_permission(EDIT_AWARDS_PERMISSION)) {
  json_not_authorized();
  exit;
}

$awardid = isset($_POST['awardid']) ? (int)$_POST['awardid'] : 0;

if ($awardid <= 0) {
  json_failure('noaward', 'No award ID specified');
  exit;
}

// Start a transaction
$db->beginTransaction();

try {
  // Get all eligible racers
  $stmt = $db->query('SELECT racerid FROM RegistrationInfo WHERE exclude = 0');
  $racers = $stmt->fetchAll(PDO::FETCH_COLUMN);
  
  // For each racer, check if they're already in this award category
  // If not, add them to it
  $check_stmt = $db->prepare('SELECT COUNT(*) FROM DesignEntries 
                             WHERE racerid = :racerid AND awardid = :awardid');
  $insert_stmt = $db->prepare('INSERT INTO DesignEntries (racerid, awardid) 
                              VALUES (:racerid, :awardid)');
  
  $racers_added = 0;
  foreach ($racers as $racerid) {
    $check_stmt->execute(array(':racerid' => $racerid, ':awardid' => $awardid));
    $exists = $check_stmt->fetchColumn();
    
    if (!$exists) {
      $insert_stmt->execute(array(
        ':racerid' => $racerid,
        ':awardid' => $awardid
      ));
      $racers_added++;
    }
  }
  
  // Commit the transaction
  $db->commit();
  
  // Return count of added racers for feedback
  json_out('racers_added', $racers_added);
  json_success();
  
} catch (Exception $e) {
  // Roll back on error
  $db->rollBack();
  json_failure('database', $e->getMessage());
}
?>
