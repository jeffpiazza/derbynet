<?php
$name = $_POST['name'];

if (have_permission(SET_UP_PERMISSION)) {
  if (read_single_value('SELECT COUNT(*) FROM Scenes WHERE name = :name',
                        array(':name' => $name)) > 0) {
    json_failure('duplicate-name', "Scene $name already exists.");
  } else {
    $stmt = $db->prepare('INSERT INTO Scenes(name, sortorder)'
                         .' VALUES(:name, 1 + (SELECT MAX(sortorder) FROM Scenes))');
    $stmt->execute(array(':name' => $name));

    json_out('scene-id', read_single_value('SELECT sceneid FROM Scenes WHERE name = :name',
                                           array(':name' => $name)));
    json_success();
  }
} else {
  json_not_authorized();
}

?>