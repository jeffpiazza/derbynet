<?php

// $_POST['full-field-calc']

if (have_permission(PRESENT_AWARDS_PERMISSION)) {
  write_raceinfo('full-field-calc', $_POST['full-field-calc']);
  json_success();
} else {
  json_not_authorized();
}

?>
