<?php
// $_POST['directory']

start_response();

$fullpath = $_POST['directory'].DIRECTORY_SEPARATOR.$_POST['basename'];

if (!have_permission(SET_UP_PERMISSION)) {
  not_authorized_body();
} else if (@mkdir($_POST['directory'])) {
  echo "<success/>";
  echo "<path>".htmlspecialchars($_POST['directory'], ENT_QUOTES, 'UTF-8')."</path>\n";
} else {
  echo "<failure code='mkdir'>Unable to create directory "
      .htmlspecialchars($_POST['directory'], ENT_QUOTES, 'UTF-8')
      ."</failure>\n";
}

end_response();

?>