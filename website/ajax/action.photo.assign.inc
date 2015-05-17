<?php
// $_POST['repo'] - photo repository name
// $_POST['racer'] - racerid of racer whose photo should be set
// $_POST['photo'] - basename of photo file; empty if removing association
require_once('inc/photo-config.inc');

$repo = photo_repository(isset($_POST['repo']) ? $_POST['repo'] : 'head');

if (isset($_POST['photo']) && strlen($_POST['photo'])) {
  $photo_path = $repo->lookup('cropped')->find_or_make_image_file($_POST['photo']);
  if (!$photo_path) {
    echo '<failure code="nosuchfile">Image '.$_POST['photo'].' not found.</failure>'."\n";
  } else {
    take_action_if_permitted('UPDATE RegistrationInfo SET '.$repo->column_name().' = :photo'
                             .' WHERE racerid = :racerid',
                             array(':photo' => $photo_path,
                                   ':racerid' => $_POST['racer']),
                             ASSIGN_RACER_IMAGE_PERMISSION);
  }
} else {
    take_action_if_permitted('UPDATE RegistrationInfo SET '.$repo->column_name().' = \'\''
                             .' WHERE racerid = :racerid',
                             array(':racerid' => $_POST['racer']),
                             ASSIGN_RACER_IMAGE_PERMISSION);
}

?>