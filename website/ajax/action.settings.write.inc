<?php
// This ajax action allows configuration information to be recorded in the database.  See setting.php.

require_once('inc/photo-config.inc');
require_once('inc/schema_version.inc');
require_once('inc/locked.inc');

// To preclude problems with misspelled parameters, we want to be sure that we
// account for every parameter passed.  We do that by just counting the number
// of parameters that were recognized, and comparing to the number of parameters
// actually passed.
$num_picked = 1;  // 1, not 0, to account for the 'action' key

if (have_permission(SET_UP_PERMISSION)) {
  // For checkbox controls, no parameter is passed if the checkbox is unchecked.
  // To avoid accidentally reseting values by forgetting to pass them, we adopt
  // a convention of sending a hidden field, named with a '-checkbox' suffix,
  // for each checkbox Boolean field.  E.g., only if a
  // 'do-use-subgroups-checkbox' parameter is passed will the presence or
  // absence of a 'do-use-subgroups' parameter be used to control the
  // 'use-subgroups' setting.
  $ok = true;
  $written = array();

  if (locked_settings()) {
    foreach(array('photo-dir', 'car-photo-dir', 'video-dir') as $post_key) {
      if (isset($_POST[$post_key])) {
        json_failure('locked', "Directory settings are locked and cannot be changed.");
        $ok = false;
      }
    }
  }

  // ===========================================================================
  // Checkbox settings: look for "-checkbox" entry, and optional $_POST key for true
  // $_POST key => RaceInfo.itemkey
  if ($ok) {
    foreach(array('do-use-subgroups' => 'use-subgroups',
                  'one-trophy-per' => 'one-trophy-per',
                  'reverse-lanes' => 'reverse-lanes',
                  'show-car-photos-on-deck' => 'show-cars-on-deck',
                  'show-car-photos-rr' => 'show-car-photos-rr',
                  'show-racer-photos-rr' => 'show-racer-photos-rr',
                  'upload-videos' => 'upload-videos',
                  'use-master-sched' => 'use-master-sched',
                  'use-points' => 'use-points',
                  'use-xbs' => 'use-xbs',
                  'warn-no-timer' => 'warn-no-timer'
              )
            as $post_key => $raceinfo_key) {
      if (isset($_POST[$post_key.'-checkbox'])) {
        ++$num_picked;
        if (isset($_POST[$post_key])) ++$num_picked;
        $written[] = $post_key;
        write_raceinfo($raceinfo_key, @$_POST[$post_key] ? '1' : '0');
      }
    }
  }

  // ===========================================================================
  // Non-checkbox settings
  // $_POST key => RaceInfo.itemkey
  if ((isset($_POST['n-lanes']) &&
       $_POST['n-lanes'] != read_raceinfo('lane_count')) ||
      (isset($_POST['unused-lane-mask']) &&
       $_POST['unused-lane-mask'] != read_raceinfo('unused-lane-mask'))) {
    if (read_single_value('SELECT COUNT(*) FROM RaceChart'
                          .' WHERE COALESCE(completed, \'\') = \'\'') > 0) {
      json_failure('already-scheduled', "Remove unraced schedules before changing lane counts.");
      $ok = false;
    }
  }
  if ($ok) {
    foreach(array('n-lanes' => 'lane_count',
                  'track-length' => 'track-length',
                  'n-pack-trophies' => 'n-pack-trophies',
                  'n-den-trophies' => 'n-den-trophies',
                  'n-rank-trophies' => 'n-rank-trophies',
                  'name-style' => 'name-style',
                  'xbs-award' => 'xbs-award',
                  'partition-label' => 'partition-label',
                  'photos-on-now-racing' => 'photos-on-now-racing',
                  'supergroup-label' => 'supergroup-label',
                  'update-period' => 'update-period',
                  'photo-dir' => 'photo-directory',
                  'car-photo-dir' => 'car-photo-directory',
                  'video-dir' => 'video-directory',
                  'log-dir' => 'logs-directory',
                  'replay-skipback' => 'replay-skipback',
                  'replay-num-showings' => 'replay-num-showings',
                  'replay-rate' => 'replay-rate',
                  'timer-send-logs' => 'timer-send-logs',
                  'unused-lane-mask' => 'unused-lane-mask',
                  'max-runs-per-car' => 'max-runs-per-car',
                  'rotation-schedule' => 'rotation-schedule',
                  'finish-formatting' => 'finish-formatting',
                  'scoring' => 'scoring',
                  'now-racing-linger-ms' => 'now-racing-linger-ms',
                  'racing_scene' => 'racing_scene',
                  'images-dir' => 'images-dir',
                  'ballot_password' => 'ballot_password',
                  'car-numbering' => 'car-numbering')
            as $post_key => $raceinfo_key) {
      if (isset($_POST[$post_key])) {
        ++$num_picked;
        $written[] = $post_key;
        write_raceinfo($raceinfo_key, $_POST[$post_key]);
        if (read_raceinfo($raceinfo_key) != $_POST[$post_key]) {
          json_failure('not-written',
                       "Value for $raceinfo_key could not be written."
                       .(schema_version() != expected_schema_version()
                         ? "<b>Upgrading the database schema may help.</b>" : ""));
          $ok = false;
        }
      }
    }
  }

  json_out('written', $written);

  if ($ok) {
    if ($num_picked == count($_POST)) {
      json_success();
    } else {
      json_failure('unrecognized', (count($_POST) - $num_picked)." unrecognized setting(s)");
    }
  }
} else {
  json_not_authorized();
}

?>
