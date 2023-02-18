<?php

require_once('inc/aggregate_round.inc');
require_once('inc/events.inc');
require_once('inc/racing-state.inc');

if (!have_permission(CONTROL_RACE_PERMISSION)) {
  json_not_authorized();
} else if (!isset($_POST['roundid'])) {
  json_failure('missing_param', 'No roundid parameter');
} else {
  $roundid = $_POST['roundid'];
  list($classid, $roundno) = read_single_row('SELECT classid, round FROM Rounds'
                                             .' WHERE roundid = :roundid',
                                             array(':roundid' => $roundid));
  if (!isset($roundno)) {
    json_failure('bad_param', 'No such round exists');
  } else if (read_single_value('SELECT COUNT(*)'
                               .' FROM RaceChart'
                               .' WHERE roundid = :roundid',
                               array(':roundid' => $roundid)) > 0) {
    json_failure('scheduled', "Heats are scheduled for this round; remove before proceeding");
  } else {
    $constituents = read_single_value('SELECT constituents FROM Classes WHERE classid = :classid',
                                      array(':classid' => $classid));
    if ($roundno == 1 && strlen($constituents) == 0) {
      json_failure('first_round', "Can't remove a first round of an organic class.");
    } else {
      $classname = read_single_value('SELECT class FROM Classes WHERE classid = :classid',
                                     array(':classid' => $classid),
                                     "(No such class)");
      record_event(EVENT_ROUND_DELETED, array('roundid' => $roundid,
                                              'classid' => $classid,
                                              'other' => "$classname, Round $roundno"));
      $stmt = $db->prepare('DELETE FROM Roster WHERE roundid = :roundid');
      $stmt->execute(array(':roundid' => $roundid));

      maybe_clean_up_aggregate_class_for_round($roundid);

      $stmt = $db->prepare('DELETE FROM Rounds WHERE roundid = :roundid');
      $stmt->execute(array(':roundid' => $roundid));
      maybe_cancel_running_round($roundid);
      json_success();
      require('ajax/query.poll.coordinator.inc');
    }
  }
}

?>
