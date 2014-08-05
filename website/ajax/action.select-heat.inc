<?php
// $_POST['now_racing']
// $_POST['roundid'] (defaults to current one)
// $_POST['heat'] (values may be numeric, 'next', or 'prev'; defaults to 'next')

require_once('inc/autoadvance.inc');
require_once('inc/racing-state.inc');

start_response();

$ok = true;

if (have_permission(CONTROL_RACE_PERMISSION)) {
  $current = get_running_round();

  if (isset($_POST['now_racing'])) {
    if (!set_racing_state($_POST['now_racing'])) {
      echo "<failure code='cant_race'>"
          ."To start racing mode, there must be a valid current heat without results."
          ."</failure>\n";
      $ok = false;
    }
  }

  if (!isset($_POST['now_racing']) || isset($_POST['roundid']) || isset($_POST['heat'])) {
    // Note: roundid may be < 0 if not previously set
    $roundid = $current['roundid'];
    $heat = 'next';

    if (isset($_POST['roundid'])) {
      $roundid = $_POST['roundid'];
      $heat = 1;
    }
    if (isset($_POST['heat'])) {
      $heat = $_POST['heat'];
    }

    if (!advance_heat($current, $roundid, $heat, /* echo_failure: */true)) {
      $ok = false;
    }
  }
} else {
  not_authorized_body();
  $ok = false;
}

if ($ok) {
  echo "<success/>\n";
}

require('ajax/query.coordinator-poll.inc');

end_response();
?>
