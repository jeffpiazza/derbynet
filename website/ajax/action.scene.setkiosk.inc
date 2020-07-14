<?php

$sceneid = $_POST['sceneid'];
$kiosk_name = $_POST['kiosk_name'];
$page = $_POST['page'];

start_response();

if (have_permission(SET_UP_PERMISSION)) {
  $prev = read_single_value('SELECT page FROM SceneKiosk'
                            .' WHERE sceneid = :sceneid'
                            .'  AND kiosk_name = :kiosk_name',
                            array(':sceneid' => $sceneid,
                                  ':kiosk_name' => $kiosk_name));
  if ($page) {
    if ($prev) {
      $stmt = $db->prepare('UPDATE SceneKiosk SET page = :page'
                           .' WHERE sceneid = :sceneid'
                           .'  AND kiosk_name = :kiosk_name');
    } else {
      $stmt = $db->prepare('INSERT INTO SceneKiosk(sceneid, kiosk_name, page)'
                           .' VALUES(:sceneid, :kiosk_name, :page)');
    }

    $stmt->execute(array(':sceneid' => $sceneid,
                         ':kiosk_name' => $kiosk_name,
                         ':page' => $page));
  } else {
    $stmt = $db->prepare('DELETE FROM SceneKiosk'
                         .' WHERE sceneid = :sceneid'
                         .'  AND kiosk_name = :kiosk_name');
    $stmt->execute(array(':sceneid' => $sceneid,
                         ':kiosk_name' => $kiosk_name));
  }

  echo "<success/>\n";
} else {
  not_authorized_body();
}

end_response();
?>