<?php
require_once('inc/xbs.inc');

$award_name = xbs_award();

if (have_permission(CHECK_IN_RACERS_PERMISSION)) {
  $ok = true;
  if ($_POST['value']) {
    $sql = 'INSERT INTO Awards(awardname, awardtypeid, racerid) VALUES(:award, 3, :racerid)';
  } else {
    $sql = 'DELETE FROM Awards WHERE racerid = :racerid AND awardname = :award';
  }
  if (take_action_silently($sql, array(':award' => $award_name,
                                       ':racerid' => $_POST['racer']))) {
    json_success();
  } else {
    json_sql_failure($sql);  // TODO Missing sql
  }
} else {
  json_not_authorized();
}
?>
