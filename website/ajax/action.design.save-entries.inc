<?php
// $_POST['racerid'] - The racer ID
// $_POST['entries'] - JSON array of {awardid, selected} objects

require_once('inc/permissions.inc');

if (!have_permission(CHECK_IN_RACERS_PERMISSION)) {
  json_not_authorized();
  exit;
}

$racerid = $_POST['racerid'];
$entries = json_decode($_POST['entries'], true);

if (!$entries || !is_array($entries)) {
  json_failure('invalid', 'Invalid entries data');
  exit;
}

// Start a transaction
$db->beginTransaction();

try {
  // First delete all existing entries for this racer
  $stmt = $db->prepare('DELETE FROM DesignEntries WHERE racerid = :racerid');
  $stmt->execute(array(':racerid' => $racerid));
  
  // Then add the new entries
  $insert_stmt = $db->prepare('INSERT INTO DesignEntries (racerid, awardid) VALUES (:racerid, :awardid)');
  
  foreach ($entries as $entry) {
    if ($entry['selected']) {
      $insert_stmt->execute(array(
        ':racerid' => $racerid,
        ':awardid' => $entry['awardid']
      ));
    }
  }
  
  // Commit the transaction
  $db->commit();
  
  json_success();
} catch (Exception $e) {
  // Roll back on error
  $db->rollBack();
  json_failure('database', $e->getMessage());
}
?>
