<?php

// $_POST['rule']: one-group or by-division

require_once('inc/divisions.inc');
require_once('inc/newracer.inc');


function form_groups_by_rule($rule) {
  global $db;
  if ($rule == 'one-group') {
    $db->exec('DELETE FROM ConstituentClasses');
    $db->exec('DELETE FROM Classes');  // TODO Aggregate classes?
    $db->exec('DELETE FROM Ranks');
    $classid = find_or_create_class('One Group');
    // Trusting that divisions have unique names
    $stmt = $db->prepare('INSERT INTO Ranks(rank, classid, sortorder)'
                         .'     SELECT name, :classid, sortorder FROM Divisions');
    $stmt->execute(array(':classid' => $classid));

    $stmt = $db->prepare('UPDATE RegistrationInfo SET classid = :classid,'
                         .' rankid = (SELECT rankid FROM Ranks'
                         .'           WHERE rank = '
                         .'            (SELECT name FROM Divisions'
                         .'                 WHERE Divisions.divisionid = RegistrationInfo.divisionid))');
    $stmt->execute(array(':classid' => $classid));
  } else if ($rule == 'by-division') {
    $db->exec('DELETE FROM ConstituentClasses');
    $db->exec('DELETE FROM Classes');  // TODO Aggregate classes
    $db->exec('DELETE FROM Ranks');

    $db->exec('INSERT INTO Classes(class, sortorder) SELECT name, sortorder FROM Divisions');
    $db->exec('UPDATE RegistrationInfo'
              .' SET classid ='
              .'   (SELECT classid FROM Classes'
              .'     WHERE class = (SELECT name FROM Divisions'
              .'                     WHERE Divisions.divisionid = RegistrationInfo.divisionid))');

    // Manufacture one rank per class
    $db->exec('INSERT INTO Ranks(rank, classid, sortorder)'
              .' SELECT class, classid, 1 FROM Classes');
    $db->exec('UPDATE RegistrationInfo'
              .' SET rankid = (SELECT rankid FROM Ranks'
              .'                WHERE Ranks.classid = RegistrationInfo.classid)');
  }
  // else ('custom') do nothing
}

if (!have_permission(CONTROL_RACE_PERMISSION)) {
  json_not_authorized();
} else if (!is_allowed_group_formation_rule($_POST['rule'])) {
  json_failure('bad-rule', 'Rule value '.$_POST['rule'].' not recognized.');
} else {
  write_group_formation_rule($_POST['rule']);

  form_groups_by_rule($_POST['rule']);

  if (isset($_POST['cleanup']) && $_POST['cleanup']) {
    clean_up_empty_classes_and_ranks();
  }

  json_success();
}

?>