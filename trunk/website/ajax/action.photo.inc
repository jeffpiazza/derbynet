<?php
require_once('inc/photo-config.inc');

$repo = photo_repository(isset($_POST['repo']) ? $_POST['repo'] : 'head');

if (have_permission(ASSIGN_RACER_IMAGE_PERMISSION) && isset($_POST['previous'])) {
  // TODO: Something is writing NULLs into ImageFile field.  Does
  // Access treat an empty string as NULL?
  take_action_silently('UPDATE RegistrationInfo SET '.$repo->column_name().' = \'\''
                       .' WHERE racerid = :racerid',
                       array(':racerid' => $_POST['previous']));
}

take_action_if_permitted('UPDATE RegistrationInfo SET '.$repo->column_name().' = :photo'
                         .' WHERE racerid = :racerid',
                         array(':photo' => $repo->lookup('display')->file_path($_POST['photo']),
                               ':racerid' => $_POST['racer']),
                         ASSIGN_RACER_IMAGE_PERMISSION);
?>