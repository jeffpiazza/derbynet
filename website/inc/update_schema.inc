<?php

require_once('inc/data.inc');
require_once('inc/schema_version.inc');
require_once('inc/partitions.inc');

function after_update_schema($original_schema) {
  global $db;
  if ($original_schema < PARTITION_SCHEMA) {
    // Updated database from older days
    $db->exec('INSERT INTO Partitions(partitionid, name, rankid, sortorder)'
              .' SELECT rankid, rank, rankid, sortorder FROM Ranks');
    $db->exec('UPDATE Partitions SET name = \''.DEFAULT_PARTITION_NAME.'\''
              .' WHERE name = \'* Not specified\'');
    $db->exec('UPDATE RegistrationInfo SET partitionid = rankid');
    record_action_partition_structure();

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
