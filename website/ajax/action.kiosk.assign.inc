<?php
// $_POST['address'] identifies the kiosk.  Kiosk names are for display to the
// user, but are not used to identify kiosks for assignment.
//
// $_POST['page'] to choose a page to display
// $_POST['params'] to add page parameters
// $_POST['name'] to assign a name for the kiosk

// Intended for demo mode:
// $_POST['all'] to choose a page to display on all kiosks
start_response();

if (have_permission(CONTROL_RACE_PERMISSION)) {
  if (isset($_POST['page'])) {
    $page = $_POST['page'];
    if (isset($_POST['params'])) {
      $page .= '#'.$_POST['params'];
    }
    $stmt = $db->prepare('UPDATE Kiosks SET page = :page WHERE address = :address');
    $stmt->execute(array(':page' => $page,
                         ':address' => $_POST['address']));
  }

  if (isset($_POST['name'])) {
    $stmt = $db->prepare('UPDATE Kiosks SET name = :name WHERE address = :address');
    $stmt->execute(array(':name' => $_POST['name'],
                         ':address' => $_POST['address']));
  }

  if (isset($_POST['all'])) {
    $stmt = $db->prepare('UPDATE Kiosks SET page = :page');
    $stmt->execute(array(':page' => $_POST['all']));
  }

  echo "<success/>\n";
} else {
  not_authorized_body();
}

end_response();
?>