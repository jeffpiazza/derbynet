<?php

// $_POST['purge'] =
//    results : Purge all race results
//    schedules
//    racers
//    awards
//
// $_POST['roundid'] may accompany results, schedules

require_once('inc/details-for-setup-page.inc');

start_response();

if (!have_permission(SET_UP_PERMISSION)) {
  not_authorized_body();
} else if ($_POST['purge'] == 'results') {
  $sql = 'UPDATE RaceChart SET finishtime = NULL, finishplace = NULL, completed = NULL';
  if (isset($_POST['roundid'])) {
    $stmt = $db->prepare($sql.' WHERE roundid = :roundid');
    $stmt->execute(array(':roundid' => $_POST['roundid']));
  } else {
    // Also: action.result.delete.inc, $_POST['roundid'] = 'all'
    $db->exec($sql);
  }
} else if ($_POST['purge'] == 'schedules') {
  $sql = 'DELETE FROM RaceChart';
  if (isset($_POST['roundid'])) {
    $stmt = $db->prepare($sql.' WHERE roundid = :roundid');
    $stmt->execute(array(':roundid' => $_POST['roundid']));
  } else {
    $db->exec($sql);
  }
} else if ($_POST['purge'] == 'racers') {
  $db->exec('UPDATE Awards SET racerid = 0');
  $db->exec('DELETE FROM RaceChart');
  $db->exec('DELETE FROM Roster');
  $db->exec('DELETE FROM RegistrationInfo');
} else if ($_POST['purge'] == 'awards') {
  $db->exec('DELETE FROM Awards');
} else {
  echo "<failure code='not-recognized'>Unrecognized purge operation: "
      .htmlspecialchars($_POST['purge'], ENT_QUOTES, 'UTF-8')
      ."</failure>";
}

echo "<success/>\n";

echo "<details><![CDATA[\n";
echo json_encode(build_setup_page_details());
echo "\n]]></details>\n";

end_response();

?>