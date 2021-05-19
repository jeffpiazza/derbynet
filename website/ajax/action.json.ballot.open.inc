<?php

// $_POST['state'] = 'open' or 'closed'

if (!have_permission(JUDGING_PERMISSION)) {
  json_not_authorized();
} else {
  write_raceinfo('balloting', $_POST['state']);
  json_success();
}
?>
