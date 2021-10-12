<?php

// $_POST['purge'] =
//    results : Purge all race results
//    schedules
//    racers
//    awards
//
// $_POST['roundid'] may accompany results, schedules

require_once('inc/details-for-setup-page.inc');
require_once('inc/fake_images.inc');
require_once('inc/events.inc');

$ok = true;
if (!have_permission(SET_UP_PERMISSION)) {
  json_not_authorized();
  $ok = false;
} else if ($_POST['purge'] == 'results') {
  record_event_purge(EVENT_PURGE_RESULTS);
  $sql = 'UPDATE RaceChart SET finishtime = NULL, finishplace = NULL, completed = NULL';
  if (isset($_POST['roundid'])) {
    $stmt = $db->prepare($sql.' WHERE roundid = :roundid');
    $stmt->execute(array(':roundid' => $_POST['roundid']));
  } else {
    // Also: action.result.delete.inc, $_POST['roundid'] = 'all'
    $db->exec($sql);
  }
} else if ($_POST['purge'] == 'schedules') {
  record_event_purge(EVENT_PURGE_SCHEDULES);
  $sql = 'DELETE FROM RaceChart';
  if (isset($_POST['roundid'])) {
    $stmt = $db->prepare($sql.' WHERE roundid = :roundid');
    $stmt->execute(array(':roundid' => $_POST['roundid']));
  } else {
    $db->exec($sql);
  }
} else if ($_POST['purge'] == 'racers') {
  record_event_purge(EVENT_PURGE_RACERS);
  $db->exec('UPDATE Awards SET racerid = 0');
  $db->exec('DELETE FROM RaceChart');
  $db->exec('DELETE FROM Roster');
  $db->exec('DELETE FROM RegistrationInfo');
  $db->exec('DELETE FROM Partitions');
  remove_fake_car_and_racer_images();
  if (read_raceinfo_boolean('fake-racers')) {
    $db->exec('DELETE FROM ConstituentClasses');
    $db->exec('DELETE FROM Ranks');
    $db->exec('DELETE FROM Classes');
  }
  delete_from_raceinfo('fake-racers');
} else if ($_POST['purge'] == 'awards') {
  record_event_purge(EVENT_PURGE_AWARDS);
  $db->exec('DELETE FROM Awards');
} else {
  json_failure('not-recognized', "Unrecognized purge operation: ".$_POST['purge']);
  $ok = false;
}

if ($ok) {
  json_success();
  json_out('details', build_setup_page_details());
}
?>