<?php
// $_POST['now_racing']
// $_POST['roundid'] (defaults to current one)
// $_POST['heat'] (values may be numeric, 'next', or 'prev'; defaults to 'next')

require_once('inc/autoadvance.inc');

start_response();

if (have_permission(CONTROL_RACE_PERMISSION)) {
  $current = get_running_round();

  if (isset($_POST['now_racing'])) {
    // TODO: CANCEL or START replay
    write_raceinfo('NowRacingState', $_POST['now_racing']);
  }

  if (!isset($_POST['now_racing']) || isset($_POST['roundid']) || isset($_POST['heat'])) {
    // Note: roundid may be < 0 if not previously set
    $roundid = $current['roundid'];
    $heat = 'next';

    if (isset($_POST['roundid'])) {
      $roundid = $_POST['roundid'];
    }
    if (isset($_POST['heat'])) {
      $heat = $_POST['heat'];
    }

    if (advance_heat($current, $roundid, $heat, true)) {
      echo '<success/>';
    }
  } else {
    echo '<success/>';
  }
} else {
  not_authorized_body();
}

require('ajax/query.coordinator-poll.inc');

end_response();
?>
