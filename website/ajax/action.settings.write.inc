<?php
// This ajax action allows configuration information to be recorded in the database.  See setting.php.

require_once('inc/photo-config.inc');
require_once('inc/schema_version.inc');
require_once('inc/locked.inc');

start_response();

// To preclude problems with misspelled parameters, we want to be sure that we
// account for every parameter passed.  We do that by just counting the number
// of parameters that were recognized, and comparing to the number of parameters
// actually passed.
$num_picked = 1;  // 1, not 0, to account for the 'action' key

if (have_permission(SET_UP_PERMISSION)) {
  // For checkbox controls, no parameter is passed if the checkbox is unchecked.
  // To avoid accidentally reseting values by forgetting to pass them, we adopt
  // a convention of sending a hidden field, named with a '-checkbox' suffix,
  // for each checkbox Boolean field.  E.g., only if a 'with-gprm-checkbox'
  // parameter is passed will the presence or absence of a 'with-gprm' parameter
  // be used to control the 'with-gprm' setting.
  $failures = false;

  if (locked_settings()) {
    foreach(array('photo-dir', 'car-photo-dir') as $post_key) {
      if (isset($_POST[$post_key])) {
        echo "<failure code='locked'>Directory settings are locked and cannot be changed.</failure>\n";
        $failures = true;
      }
    }
  }

  if (!$failures) {
    // Checkbox settings: look for "-checkbox" entry, and optional $_POST key for true
    // $_POST key => RaceInfo.itemkey
    foreach(array('with-gprm' => 'with-gprm',
                  'do-use-subgroups' => 'use-subgroups',
                  'use-master-sched' => 'use-master-sched',
                  'show-racer-photos' => 'show-racer-photos',
                  'show-car-photos-on-deck' => 'show-cars-on-deck',
                  'show-racer-photos-rr' => 'show-racer-photos-rr',
                  'show-car-photos-rr' => 'show-car-photos-rr',
                  'drop-slowest' => 'drop-slowest')
            // See also use-xbs, below
            as $post_key => $raceinfo_key) {
      if (isset($_POST[$post_key.'-checkbox'])) {
        ++$num_picked;
        if (isset($_POST[$post_key])) ++$num_picked;
        echo "<wrote>".$post_key."</wrote>\n";
        write_raceinfo($raceinfo_key, @$_POST[$post_key] ? '1' : '0');
      }
    }

    // $_POST key => RaceInfo.itemkey
    foreach(array('n-lanes' => 'lane_count',
                  'track-length' => 'track-length',
                  'n-pack-trophies' => 'n-pack-trophies',
                  'n-den-trophies' => 'n-den-trophies',
                  'n-rank-trophies' => 'n-rank-trophies',
                  'name-style' => 'name-style',
                  'xbs-award' => 'xbs-award',  // @$_POST['use-xbs'] ? $_POST['xbs-award'] : ''
                  'group-label' => 'group-label',
                  'subgroup-label' => 'subgroup-label',
                  'supergroup-label' => 'supergroup-label',
                  'update-period' => 'update-period',
                  'photo-dir' => 'photo-directory',
                  'car-photo-dir' => 'car-photo-directory',
                  'replay-skipback' => 'replay-skipback',
                  'replay-num-showings' => 'replay-num-showings',
                  'replay-rate' => 'replay-rate',
                  'unused-lane-mask' => 'unused-lane-mask')
            as $post_key => $raceinfo_key) {
      if (isset($_POST[$post_key])) {
        ++$num_picked;
        echo "<wrote>".$post_key."</wrote>\n";
        write_raceinfo($raceinfo_key, $_POST[$post_key]);
        if (read_raceinfo($raceinfo_key) != $_POST[$post_key]) {
          echo "<failure code='not-written'>Value for ".$raceinfo_key." could not be written.";
          if (schema_version() != expected_schema_version()) {
            echo "<b>Upgrading the database schema may help.</b>";
          }
          echo "</failure>\n";
          $failures = true;
        }
      }
    }
  }

  if (isset($_POST['use-xbs-checkbox'])) {
    ++$num_picked;
    if (isset($_POST['use-xbs'])) ++$num_picked;
    if (!@$_POST['use-xbs']) {
      write_raceinfo('xbs-award', '');
    }
  }

  if ($num_picked == count($_POST)) {
    // If $failures is set, then at least one of the settings had a problem, and
    // we shouldn't claim success
    if (!$failures) {
      echo "<success/>\n";
    }
  } else {
    echo "<failure code='unrecognized'>".(count($_POST) - $num_picked)." unrecognized setting(s)</failure>\n";
  }
} else {
  not_authorized_body();
}

end_response();

?>