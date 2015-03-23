<?php
// $_POST['repo'] - photo repository name
// $_POST['racer'] - racerid of racer whose photo should be set
// $_POST['photo'] - basename of photo file; empty if removing association
require_once('inc/photo-config.inc');

$repo = photo_repository(isset($_POST['repo']) ? $_POST['repo'] : 'head');

$photo_path = isset($_POST['photo']) && strlen($_POST['photo']) > 0
    ? $repo->lookup('display')->file_path($_POST['photo']) : '';

take_action_if_permitted('UPDATE RegistrationInfo SET '.$repo->column_name().' = :photo'
                         .' WHERE racerid = :racerid',
                         array(':photo' => $photo_path,
                               ':racerid' => $_POST['racer']),
                         ASSIGN_RACER_IMAGE_PERMISSION);
?>