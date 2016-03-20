<?php

// Set "current" award being presented

// $_POST['key'] is one of:
// award-<awardid>
// speed-<place>
// speed-<place>-<den>

start_response();

require_once('inc/photo-config.inc');
require_once('inc/standings.inc');
require_once('inc/ordinals.inc');

if (have_permission(PRESENT_AWARDS_PERMISSION)) {
  if (isset($_POST['key'])) {
    $key = $_POST['key'];
    write_raceinfo('current_award_key', $key);
    echo "<success/>\n";
  }
} else {
  not_authorized_body();
}

end_response();
?>
