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

  $all_settings = array(
    'ballot_password' => array('setting' => 'ballot_password'),
    'car-numbering' => array('setting' => 'car-numbering'),
    'car-photo-dir' => array('setting' => 'car-photo-directory'),
    'do-use-subgroups' => array('setting' => 'use-subgroups', 'type' => 'checkbox'),
    'finish-formatting' => array('setting' => 'finish-formatting'),
    'images-dir' => array('setting' => 'images-dir'),
    'log-dir' => array('setting' => 'logs-directory'),
    'max-runs-per-car' => array('setting' => 'max-runs-per-car', 'type' => 'int'),
    'n-den-trophies' => array('setting' => 'n-den-trophies', 'type' => 'int'),
    'n-lanes' => array('setting' => 'lane_count', 'type' => 'int'),
    'n-pack-trophies' => array('setting' => 'n-pack-trophies', 'type' => 'int'),
    'n-rank-trophies' => array('setting' => 'n-rank-trophies', 'type' => 'int'),
    'name-style' => array('setting' => 'name-style'),
    'now-racing-linger-ms' => array('setting' => 'now-racing-linger-ms'),
    'one-trophy-per' => array('setting' => 'one-trophy-per', 'type' => 'checkbox'),
    'partition-label' => array('setting' => 'partition-label'),
    'photo-dir' => array('setting' => 'photo-directory'),
    'photos-on-now-racing' => array('setting' => 'photos-on-now-racing'),
    'racing_scene' => array('setting' => 'racing_scene'),
    'replay-num-showings' => array('setting' => 'replay-num-showings', 'type' => 'int'),
    'replay-rate' => array('setting' => 'replay-rate', 'type' => 'number'),
    'replay-skipback' => array('setting' => 'replay-skipback', 'type' => 'number'),
    'reverse-lanes' => array('setting' => 'reverse-lanes', 'type' => 'checkbox'),
    'rotation-schedule' => array('setting' => 'rotation-schedule'),
    'scoring' => array('setting' => 'scoring'),
    'show-car-photos-on-deck' => array('setting' => 'show-cars-on-deck', 'type' => 'checkbox'),
    'show-car-photos-rr' => array('setting' => 'show-car-photos-rr', 'type' => 'checkbox'),
    'show-racer-photos-rr' => array('setting' => 'show-racer-photos-rr', 'type' => 'checkbox'),
    'supergroup-label' => array('setting' => 'supergroup-label'),
    'timer-send-logs' => array('setting' => 'timer-send-logs'),
    'track-length' => array('setting' => 'track-length', 'type' => 'number'),
    'unused-lane-mask' => array('setting' => 'unused-lane-mask'),
    'update-period' => array('setting' => 'update-period'),
    'upload-videos' => array('setting' => 'upload-videos', 'type' => 'checkbox'),
    'use-master-sched' => array('setting' => 'use-master-sched', 'type' => 'checkbox'),
    'use-points' => array('setting' => 'use-points', 'type' => 'checkbox'),
    'use-xbs' => array('setting' => 'use-xbs', 'type' => 'checkbox'),
    'video-dir' => array('setting' => 'video-directory'),
    'warn-no-timer' => array('setting' => 'warn-no-timer', 'type' => 'checkbox'),
    'xbs-award' => array('setting' => 'xbs-award'),
  );

  if ($ok) {
    foreach ($all_settings as $post_key_base => $setting) {
      if (@$setting['type'] == 'checkbox') {
        $post_key = $post_key_base.'-checkbox';
      } else {
        $post_key = $post_key_base;
      }
      if (isset($_POST[$post_key])) {
        $itemkey = $setting['setting'];
        $itemvalue = $_POST[$post_key];
        ++$num_picked;
        $written[] = $post_key;
        if (@$setting['type'] == 'checkbox') {
          if (isset($_POST[$post_key_base])) {
            ++$num_picked;
            $itemvalue = '1';
          } else {
            $itemvalue = '0';
          }
        } else if (@$setting['type'] == 'int') {
          // Completely non-numeric values become zero here, otherwise '123x' becomes 123, etc.
          $itemvalue = strval(intval($itemvalue));
        } else if (@$setting['type'] == 'number') {
          if (!is_numeric($itemvalue)) {
            $itemvalue = '0.0';
          }
        }
        write_raceinfo($itemkey, $itemvalue);
        // Check that the value didn't get truncated or something
        if (read_raceinfo($itemkey) != $itemvalue) {
          json_failure('not-written',
                       "Value for $itemkey could not be written."
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
