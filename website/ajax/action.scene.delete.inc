<?php
$sceneid = $_POST['sceneid'];

start_response();

if (have_permission(SET_UP_PERMISSION)) {
  $stmt = $db->prepare('DELETE FROM SceneKiosk WHERE sceneid = :sceneid');
  $stmt->execute(array(':sceneid' => $sceneid));

  $stmt = $db->prepare('DELETE FROM Scenes WHERE sceneid = :sceneid');
  $stmt->execute(array(':sceneid' => $sceneid));

  echo "<success/>\n";
} else {
  not_authorized_body();
}

end_response();

?>