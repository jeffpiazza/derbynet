<?php
// This action allows manual selection of the current racing heat.
//
// $_POST['now_racing']
// $_POST['roundid'] (defaults to current one)
// $_POST['heat'] (values may be numeric, 'next', or 'prev'; defaults to 'next')
//
// Use cases:
//
// "Racing/Not Racing" flipswitch: $_POST['now_racing'], only.
//
// "Skip Heat"/"Previous Heat" buttons: $_POST['heat'] = 'next' or 'prev'
//
// "Race" button (one per roundid): $_POST['roundid'], { $_POST['heat']=1 }, $_POST['now_racing']=1
//   now_racing is presumed to apply AFTER the new race heat is
//   selected.  If not possible to engage now_racing, still considered
//   a success.
//
// "Select Next" (for master scheduling): $_POST['heat']=next-up

require_once('inc/autoadvance.inc');
require_once('inc/racing-state.inc');
require_once('inc/scenes.inc');

$ok = true;

if (have_permission(CONTROL_RACE_PERMISSION)) {
  if (isset($_POST['roundid']) || isset($_POST['heat'])) {
    // Turn off NowRacing, although it may get turned back on before
    // this action completes.  set_racing_state(0) will take care of
    // sending a replay CANCEL.
    set_racing_state(0);

    $current = get_running_round();

    if (isset($_POST['roundid'])) {
      $current['roundid'] = $_POST['roundid'];
      $heat = 1;
    }
    if (isset($_POST['heat'])) {
      $heat = $_POST['heat'];
    }

    if (!advance_heat($current, $heat, /* report_failure: */'json')) {
      $ok = false;
    }
  }

  if ($ok && isset($_POST['now_racing'])) {
    if (set_racing_state($_POST['now_racing'])) {
      if ($_POST['now_racing']) {
        maybe_apply_racing_scene();
      }
    } else {
      // This 'if' test is to control the wording of the failure
      // message from clicking the 'race' button for a heat whose
      // first result already has data.
      $ok = false;
      if (isset($_POST['roundid'])) {
        json_failure('cant_race',
                     "The first heat in this group has already been run.  "
                     + "Please choose a heat without results.");
      } else {
        // Clean up refused-results, even if the attempt was unsuccessful.
        write_raceinfo('refused-results', 0);
        json_failure('cant_race',
                     "To start racing mode, there must be a valid current heat without results.");
      }
    }
  }
} else {
  json_not_authorized();
  $ok = false;
}

if ($ok) {
  json_success();
}

require('ajax/query.poll.coordinator.inc');
?>
