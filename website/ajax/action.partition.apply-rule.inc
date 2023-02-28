<?php

// $_POST['rule']: 'one-group' or 'by-partition' (or 'custom')


require_once('inc/partitions.inc');
require_once('inc/newracer.inc');
require_once('inc/events.inc');

function form_groups_by_rule($rule) {
  global $db;
  if ($rule == 'one-group') {
    $db->exec('DELETE FROM Roster');
    $db->exec('DELETE FROM Rounds');
    $db->exec('DELETE FROM ConstituentClasses');
    $db->exec('DELETE FROM Awards WHERE rankid IS NOT NULL OR classid IS NOT NULL');
    $db->exec('DELETE FROM Classes');
    $db->exec('DELETE FROM Ranks');
    // find_or_create_class will create a Rounds entry, if necessary.
    $classid = find_or_create_class('All Racers');
    // Trusting that partitions have unique names
    $stmt = $db->prepare('INSERT INTO Ranks(rank, classid, sortorder)'
                         .'     SELECT name, :classid, sortorder FROM Partitions');
    $stmt->execute(array(':classid' => $classid));

    $stmt = $db->prepare('UPDATE RegistrationInfo SET classid = :classid,'
                         .' rankid = (SELECT rankid FROM Ranks'
                         .'           WHERE rank = '
                         .'            (SELECT name FROM Partitions'
                         .'                 WHERE Partitions.partitionid = RegistrationInfo.partitionid))');
    $stmt->execute(array(':classid' => $classid));

    // Update Partitions table's rankid values to match new ranks
    $db->exec('UPDATE Partitions'
              .' SET rankid = (SELECT rankid FROM Ranks'
              .'               WHERE Ranks.rank = Partitions.name)');

    fill_in_missing_roster_entries();
  } else if ($rule == 'by-partition') {
    $db->exec('DELETE FROM Roster');
    $db->exec('DELETE FROM Rounds');
    $db->exec('DELETE FROM ConstituentClasses');
    $db->exec('DELETE FROM Awards WHERE rankid IS NOT NULL OR classid IS NOT NULL');
    $db->exec('DELETE FROM Classes');
    $db->exec('DELETE FROM Ranks');

    $db->exec('INSERT INTO Classes(class, sortorder) SELECT name, sortorder FROM Partitions');
    $db->exec('UPDATE RegistrationInfo'
              .' SET classid ='
              .'   (SELECT classid FROM Classes'
              .'     WHERE class = (SELECT name FROM Partitions'
              .'                     WHERE Partitions.partitionid = RegistrationInfo.partitionid))');

    // Manufacture one rank per class
    $db->exec('INSERT INTO Ranks(rank, classid, sortorder)'
              .' SELECT class, classid, 1 FROM Classes');
    $db->exec('UPDATE RegistrationInfo'
              .' SET rankid = (SELECT rankid FROM Ranks'
              .'                WHERE Ranks.classid = RegistrationInfo.classid)');

    // Update Partitions table's rankid values to match new ranks
    $db->exec('UPDATE Partitions'
              .' SET rankid = (SELECT rankid FROM Ranks'
              .'               WHERE Ranks.rank = Partitions.name)');

    // Add racing rounds for the new groups.
    $db->exec('INSERT INTO Rounds(round, classid) SELECT 1, classid FROM Classes');
    fill_in_missing_roster_entries();
  }
  // else ('custom') do nothing
}

if (!have_permission(CONTROL_RACE_PERMISSION)) {
  json_not_authorized();
} else if (!is_allowed_group_formation_rule($_POST['rule'])) {
  json_failure('bad-rule', 'Rule value '.$_POST['rule'].' not recognized.');
} else if (read_single_value('SELECT COUNT(*) FROM RaceChart'
                             .' WHERE finishtime IS NOT NULL OR finishplace IS NOT NULL')
           > 0) {
  json_failure('results-exist', 'Racing results already exist.');
} else {
  record_event(EVENT_GROUP_FORMATION_RULE_APPLIED,
               array('other' => group_formation_rule().' -> '.$_POST['rule']));
  write_group_formation_rule($_POST['rule']);

  form_groups_by_rule($_POST['rule']);

  clean_up_empty_classes_and_ranks();

  write_raceinfo('use-subgroups', ($_POST['rule'] == 'by-partition') ? 0 : 1);

  json_success();
}

?>
