<?php
// This ajax action allows configuration information to be recorded in the database.  See setting.php.

require_once('inc/photo-config.inc');

start_response();

$num_picked = 1;  // 1, not 0, to account for the 'action' key

// TODO: These are checkboxes, and won't be passed if the checkbox is unchecked.
if (have_permission(SET_UP_PERMISSION)) {
  foreach(array('with-gprm' => 'with-gprm',
                'do-use-subgroups' => 'use-subgroups',
                'use-master-sched' => 'use-master-sched',
                'show-racer-photos' => 'show-racer-photos',
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

  foreach(array('n-lanes' => 'lane_count',
                'track-length' => 'track-length',
                'n-den-trophies' => 'n-den-trophies',
                'n-pack-trophies' => 'n-pack-trophies',
                'xbs-award' => 'xbs-award',  // @$_POST['use-xbs'] ? $_POST['xbs-award'] : ''
                'group-label' => 'group-label',
                'subgroup-label' => 'subgroup-label',
                'supergroup-label' => 'supergroup-label',
                'update-period' => 'update-period',
                'photo-dir' => 'photo-directory',
                'car-photo-dir' => 'car-photo-directory')
          as $post_key => $raceinfo_key) {
    if (isset($_POST[$post_key])) {
      ++$num_picked;
      echo "<wrote>".$post_key."</wrote>\n";
      write_raceinfo($raceinfo_key, $_POST[$post_key]);
    }
  }

  if (isset($_POST['photo-width']) && isset($_POST['photo-height'])) {
    $num_picked += 2;
    $old_photo_size = read_raceinfo('photo-size');
    $new_photo_size = $_POST['photo-width'].'x'.$_POST['photo-height'];
    write_raceinfo('photo-size', $new_photo_size);
    if ($old_photo_size != $new_photo_size) {
      headshots()->lookup('display')->wipe_directory();
      echo "<wiped-display-directory/>\n";
    }
  }

  if (isset($_POST['car-photo-width']) && isset($_POST['car-photo-height'])) {
    $num_picked += 2;
    $old_photo_size = read_raceinfo('car-photo-size');
    $new_photo_size = $_POST['car-photo-width'].'x'.$_POST['car-photo-height'];
    write_raceinfo('car-photo-size', $new_photo_size);
    if ($old_photo_size != $new_photo_size) {
      car_photo_repository()->lookup('display')->wipe_directory();
      echo "<wiped-display-directory/>\n";
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
    echo "<success/>\n";
  } else {
    echo "<failure code='unrecognized'>".(count($_POST) - $num_picked)." unrecognized setting(s)</failure>\n";
  }
} else {
  not_authorized_body();
}

end_response();

?>