<?php
require_once('inc/permissions.inc');

if (!have_permission(CHECK_IN_RACERS_PERMISSION) && !have_permission(EDIT_AWARDS_PERMISSION)) {
  json_not_authorized();
  exit;
}

$racerid = $_POST['racerid'];
$awardid = $_POST['awardid'];
$selected = $_POST['selected'];

try {
  if ($selected === 'true' || $selected === '1') {
    $stmt = $db->prepare('INSERT OR IGNORE INTO DesignEntries (racerid, awardid) VALUES (:racerid, :awardid)');
    $stmt->execute(array(':racerid' => $racerid, ':awardid' => $awardid));
  } else {
    $stmt = $db->prepare('DELETE FROM DesignEntries WHERE racerid = :racerid AND awardid = :awardid');
    $stmt->execute(array(':racerid' => $racerid, ':awardid' => $awardid));
  }
  json_success();
} catch (Exception $e) {
  json_failure('database', $e->getMessage());
}
?>
