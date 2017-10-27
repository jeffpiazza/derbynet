<?php
/*
 Required: awardid ('new' for new award)
  racerid
*/
if (have_permission(JUDGING_PERMISSION)) {
  start_response();

  try {
    $stmt = $db->prepare('UPDATE Awards SET racerid = :racerid WHERE awardid = :awardid');
    $stmt->execute(array(':awardid' => $_POST['awardid'],
                         ':racerid' => $_POST['racerid']));
    echo "<success/>\n";
    require('ajax/query.award.list.inc');
  } catch (Exception $e) {
    echo "<failure code='sql'>".htmlspecialchars($e->getMessage(), ENT_QUOTES, 'UTF-8')."</failure>\n";
  }
  end_response();
} else {
  not_authorized();
}
?>