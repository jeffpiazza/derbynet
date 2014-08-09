<?php
// $_POST['address'] identifies the kiosk
// $_POST['page'] to choose a page to display
// $_POST['name'] to assign a name for the kiosk

start_response();

if (have_permission(CONTROL_RACE_PERMISSION)) {
  if (isset($_POST['page'])) {
    $stmt = $db->prepare('UPDATE Kiosks SET page = :page WHERE address = :address');
    $stmt->execute(array(':page' => $_POST['page'],
                         ':address' => $_POST['address']));
  }

  if (isset($_POST['name'])) {
    $stmt = $db->prepare('UPDATE Kiosks SET name = :name WHERE address = :address');
    $stmt->execute(array(':name' => $_POST['name'],
                         ':address' => $_POST['address']));
  }

  echo "<success/>\n";
} else {
  not_authorized_body();
}

end_response();
?>