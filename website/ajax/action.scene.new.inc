<?php
$name = $_POST['name'];

start_response();

if (have_permission(SET_UP_PERMISSION)) {
  if (read_single_value('SELECT COUNT(*) FROM Scenes WHERE name = :name',
                        array(':name' => $name)) > 0) {
    echo "<failure code='duplicate_name'>Scene $name already exists.</failure>";
  } else {
    $stmt = $db->prepare('INSERT INTO Scenes(name, sortorder)'
                         .' VALUES(:name, 1 + (SELECT MAX(sortorder) FROM Scenes))');
    $stmt->execute(array(':name' => $name));
    echo "<success/>\n";
    $sceneid = read_single_value('SELECT sceneid FROM Scenes WHERE name = :name',
                                 array(':name' => $name));
    echo "<scene id=\"$sceneid\"/>\n";
  }
} else {
  not_authorized_body();
}

end_response();

?>