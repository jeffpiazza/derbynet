<?php

// $_POST['classid'] -- the classid of the class to delete.
//
// Deletes all the ranks within the class, as well as all race results and scheduled rounds for the class.
require_once('inc/json-classes.inc');
require_once('inc/schema_version.inc');
require_once('inc/events.inc');

$classid = $_POST['classid'];

if (have_permission(SET_UP_PERMISSION)) {
  try {
    $ok = true;
    $schema_version = schema_version();
    
    if ($ok) {
      $nracers = read_single_value('SELECT COUNT(*) FROM RegistrationInfo WHERE classid = :classid',
                                   array(':classid' => $classid));
      if ($nracers > 0) {
        json_failure('existing-racers', "Unable to delete a class with racers");
        $ok = false;
      }
    }
    if ($ok) {
      if ($schema_version >= PARTITION_SCHEMA) {
        if (read_single_value('SELECT COUNT(*) FROM ConstituentClasses WHERE antecedent = :classid',
                              array(':classid' => $classid)) > 0) {
          json_failure('constituent', "Unable to delete a constituent class");
          $ok = false;
        }
      } else if ($schema_version >= 3) {
        $stmt = $db->prepare('SELECT class, constituents FROM Classes');
        $stmt->execute(array());
        foreach ($stmt as $row) {
          if (in_array($classid, explode(',', $row['constituents']))) {
            json_failure('constituent', "Unable to delete a constituent class");
            $ok = false;
          }
        }
      }
    }

    if ($ok) {
      delete_class($classid);

      json_out('classes', json_all_classes());
      json_success();
    }
  } catch (Exception $e) {
    json_failure('sql', $e->getMessage());
  }
} else {
  json_not_authorized();
}
?>