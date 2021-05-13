<?php
$award_name = read_raceinfo('xbs-award', 'Exclusively By Scout');

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
    $info = $db->errorInfo();
    json_failure('sql'.$info[0].'-'.$info[1],
                 $sql.' failed: '.$info[2]);
  }
} else {
  json_not_authorized();
}
?>