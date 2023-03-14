<?php

// $_POST['pack-aggregate-id']

if (have_permission(PRESENT_AWARDS_PERMISSION)) {
  write_raceinfo('pack-aggregate-id', $_POST['pack-aggregate-id']);
  json_success();
} else {
  json_not_authorized();
}

?>
