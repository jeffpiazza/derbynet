<?php

// $_POST['state'] = 'open' or 'closed'

start_response();
if (!have_permission(JUDGING_PERMISSION)) {
  not_authorized_body();
} else {
  write_raceinfo('balloting', $_POST['state']);
  echo "\n<success/>\n";
}
end_response();
?>
