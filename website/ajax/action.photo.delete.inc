<?php
// $_POST['repo'] - photo repository name
// $_POST['photo'] - base name of photo file
require_once('inc/photo-config.inc');
require_once('inc/events.inc');


start_response();
echo "\n";

if (have_permission(ASSIGN_RACER_IMAGE_PERMISSION)) {
  $repo = photo_repository(isset($_POST['repo']) ? $_POST['repo'] : 'head');
  foreach ($repo->all_renders() as &$render) {
    $photo_path = $render->file_path($_POST['photo']);
    if (file_exists($photo_path)) {
      @unlink($photo_path);
    }
  }

  $basename = basename($_POST['photo']);
  $column = $repo->column_name();
  $racers_to_unassign = array();
  try {  // $column may not be a defined column
    $stmt = $db->prepare('SELECT racerid, '.$column.' AS photo FROM RegistrationInfo'
                       .' WHERE '.$column.' LIKE :imagefile');
    $stmt->execute(array(':imagefile' => '%'.$basename));
    foreach ($stmt as $row) {
      if (basename($row['photo']) == $basename) {
        $racers_to_unassign[] = $row['racerid'];
      }
    }
    if (!empty($racers_to_unassign)) {
      $db->exec("UPDATE RegistrationInfo SET $column = ''"
                ." WHERE racerid IN (".implode(',', $racers_to_unassign).")");
    }
  } catch (Exception $e) {
  }
  echo "<success/>\n";
} else {
  not_authorized_body();
}

end_response();

?>