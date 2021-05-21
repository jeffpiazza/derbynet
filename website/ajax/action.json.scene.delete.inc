<?php
$sceneid = $_POST['sceneid'];

if (have_permission(SET_UP_PERMISSION)) {
  $stmt = $db->prepare('DELETE FROM SceneKiosk WHERE sceneid = :sceneid');
  $stmt->execute(array(':sceneid' => $sceneid));

  $stmt = $db->prepare('DELETE FROM Scenes WHERE sceneid = :sceneid');
  $stmt->execute(array(':sceneid' => $sceneid));

  json_success();
} else {
  json_not_authorized();
}

?>