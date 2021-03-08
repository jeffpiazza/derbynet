<?php

require_once('inc/aggregate_round.inc');
require_once('inc/events.inc');

start_response();

if (!have_permission(CONTROL_RACE_PERMISSION)) {
  not_authorized_body();
} else if (!isset($_POST['roundid'])) {
  echo "<failure code='missing_param'>No roundid parameter</failure>\n";
} else {
  $roundno = read_single_value('SELECT round'
                               .' FROM Rounds'
                               .' WHERE roundid = :roundid',
                               array(':roundid' => $_POST['roundid']));
  if ($roundno === false) {
    echo "<failure code='bad_param'>No such round exists</failure>\n";
  // } else if ($roundno == 1) {
  //    echo "<failure code='first_round'>Can't remove a first round.</failure>\n";
  } else if (read_single_value('SELECT COUNT(*)'
                               .' FROM RaceChart'
                               .' WHERE roundid = :roundid',
                               array(':roundid' => $_POST['roundid'])) > 0) {
    echo "<failure code='scheduled'>Heats are scheduled for this round; remove before proceeding</failure>\n";
  } else {
    $roundid = $_POST['roundid'];
    record_event_delete_round(-1, $roundid);
    $stmt = $db->prepare('DELETE FROM Roster WHERE roundid = :roundid');
    $stmt->execute(array(':roundid' => $roundid));

    maybe_clean_up_aggregate_class_for_round($roundid);

    $stmt = $db->prepare('DELETE FROM Rounds WHERE roundid = :roundid');
    $stmt->execute(array(':roundid' => $roundid));
    echo "<success/>\n";
    require('ajax/query.poll.coordinator.inc');
  }
}

end_response();
?>