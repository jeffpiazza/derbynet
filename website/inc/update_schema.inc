<?php

require_once('inc/data.inc');
require_once('inc/schema_version.inc');

function after_update_schema($original_schema) {
  global $db;
  if ($original_schema < DIVISIONS_SCHEMA) {
    // Updated database from older days
    // divisionid, name, superdiv, sortorder
    $db->exec('INSERT INTO Divisions(divisionid, name, sortorder)'
              .' SELECT classid, class, sortorder FROM Classes');
    $db->exec('UPDATE Divisions SET name = \'Default\''
              .' WHERE name = \'* Not specified\'');
    $db->exec('UPDATE RegistrationInfo SET divisionid = classid');

    $db->exec('INSERT INTO Subdivisions(name, superdiv, sortorder)'
              .' SELECT rank, classid, sortorder FROM Ranks');
    $db->exec('UPDATE Subdivisions SET name = \'Default\''
              .' WHERE name = \'* Not specified\'');
    $db->exec('UPDATE RegistrationInfo SET subdivisionid = rankid');

    $stmt = $db->prepare('SELECT classid, constituents FROM Classes WHERE constituents <> \'\'');
    $stmt->execute();

    $ins = $db->prepare('INSERT INTO ConstituentClasses(antecedent, dependent) VALUES(:ante, :dep)');
    foreach ($stmt as $row) {
      $dep = $row['classid'];
      foreach (explode(',', $row['constituents']) as $ante) {
        $ins->execute(array(':ante' => $ante, ':dep' => $dep));
      }
    }
  }
}

?>