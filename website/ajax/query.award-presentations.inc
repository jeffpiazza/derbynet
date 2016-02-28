<award-presentations>
<?php

// "Current" award being presented, and whether it's revealed

require_once('inc/photo-config.inc');
require_once('inc/standings.inc');
require_once('inc/ordinals.inc');

if (isset($_GET['key'])) {
  $key = $_GET['key'];
  // TODO: This is just to facilitate debugging until there's a proper awards dashboard.
  write_raceinfo('current_award_key', $key);
} else {
  $key = read_raceinfo('current_award_key', '');
}

// award-<awardid>
// speed-<place>
// speed-<place>-<den>
$keys = explode('-', $key);

if (count($keys) >= 2) {
  if ($keys[0] == 'award') {
    $awardid = $keys[1];
    $stmt = $db->prepare('SELECT awardname, carnumber, carname,'
                         .' firstname, lastname, imagefile, carphoto'
                         .' FROM '.inner_join('Awards', 'RegistrationInfo',
                                              'Awards.racerid = RegistrationInfo.racerid')
                         .' WHERE awardid = :awardid');
    $stmt->execute(array(':awardid' => $awardid));
  } else if ($keys[0] == 'speed') {
    // 1st place is 1, 2nd place is 2, etc.
    $place = $keys[1];
    if (count($keys) == 3) {
      $classid = $keys[2];
      $by_class = top_finishers_by_class($place);
      $racerid = $by_class[$classid][$place - 1];
      $classname = read_single_value('SELECT class FROM Classes WHERE classid = :classid',
                                     array(':classid' => $classid));
      $awardname = ($place == 1 ? '' : (ordinal($place).' ')).'Fastest in '. /* group_label().' '. */ $classname;
    } else {
      $finishers = top_finishers_overall($place);
      $racerid = $finishers[$place - 1];
      $awardname = ($place == 1 ? '' : (ordinal($place).' ')).'Fastest in '.supergroup_label();
    }

    if ($racerid > 0) {
      $stmt = $db->prepare('SELECT :awardname AS awardname, carnumber, carname,'
                           .' firstname, lastname, imagefile, carphoto'
                           .' FROM RegistrationInfo'
                           .' WHERE racerid = :racerid');
      $stmt->execute(array(':racerid' => $racerid,
                           ':awardname' => $awardname));
    }
  } else {
    echo "<failure code=\"unrecognized-award-code\">Unrecognized award code</failure>\n";
  }
} else {
  echo "<failure code=\"no-current-award\"/>\n";
}

if (isset($stmt)) {
  foreach ($stmt as $row) {
    echo '<award key="'.$key.'"'
      .' awardname="'.htmlspecialchars($row['awardname'], ENT_QUOTES, 'UTF-8').'"'
      // TODO .' revealed="'.htmlspecialchars($row['awardname'], ENT_QUOTES, 'UTF-8').'"'
      .' carnumber="'.htmlspecialchars($row['carnumber'], ENT_QUOTES, 'UTF-8').'"'
      .' carname="'.htmlspecialchars($row['carname'], ENT_QUOTES, 'UTF-8').'"'
      .' firstname="'.htmlspecialchars($row['firstname'], ENT_QUOTES, 'UTF-8').'"'
      .' lastname="'.htmlspecialchars($row['lastname'], ENT_QUOTES, 'UTF-8').'"'
    .' headphoto="'.headshots()->url_for_racer($row, 'cropped').'"'
    .' carphoto="'.car_photo_repository()->url_for_racer($row, 'cropped').'"'
      .'/>'."\n";
  }
}

?></award-presentations>
