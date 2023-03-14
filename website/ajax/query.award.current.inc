<?php

// "Current" award being presented, and whether it's revealed
// Cold-cache performance on Mac is ~75ms

require_once('inc/photo-config.inc');
require_once('inc/standings.inc');
require_once('inc/ordinals.inc');
require_once('inc/awards.inc');
require_once('inc/name-mangler.inc');

$key = read_raceinfo('current_award_key', '');
$use_subgroups = use_subgroups();
$reveal = read_raceinfo_boolean('reveal_current_award');
$name_style = read_name_style();

// award-<awardid>
// speed-<place>
// speed-<place>-<den>
// speed-<place>-<den>-<rank>
$keys = explode('-', $key);

if (count($keys) >= 2) {
  if ($keys[0] == 'award') {
    $awardid = $keys[1];
    $stmt = $db->prepare('SELECT Awards.racerid, awardname, firstname, lastname,'
                         .' carnumber, carname, class, rank, imagefile, carphoto'
                         .' FROM '.inner_join('Awards', 'RegistrationInfo',
                                              'Awards.racerid = RegistrationInfo.racerid',
                                              'Classes', 'Classes.classid = RegistrationInfo.classid',
                                              'Ranks', 'Ranks.rankid = RegistrationInfo.rankid')
                         .' WHERE awardid = :awardid');
    $stmt->execute(array(':awardid' => $awardid));
  } else if ($keys[0] == 'speed') {
    
    // 1st place is 1, 2nd place is 2, etc.
    // In case of ties, 1a means first first-place award, 1b second first-place award, etc.
    $place = $keys[1];
    $place_last_char = $place[strlen($place) - 1];
    if ('a' <= $place_last_char && $place_last_char <= 'z') {
      $place_index = ord($place_last_char) - ord('a');
      $place = intval(substr($place, 0, strlen($place) - 1));
    } else {
      $place_index = 0;
    }

    $awards = array();
    add_speed_awards($awards);

    // json_out('speed-awards', $awards);
    // json_out('keys', $keys);  // speed-1-6

    if (count($keys) == 4) {
      $rankid = $keys[3];
      $rankname = read_single_value('SELECT rank FROM Ranks WHERE rankid = :rankid',
                                    array(':rankid' => $rankid));
      $awardname = nth_fastest($place, $rankname);
    } else if (count($keys) == 3) {
      $classid = $keys[2];
      $pack_aggregate_id = read_raceinfo('pack-aggregate-id', 0);
      if ($classid != $pack_aggregate_id) {
        $classname = read_single_value('SELECT class FROM Classes WHERE classid = :classid',
                                       array(':classid' => $classid));
        $awardname = nth_fastest($place, $classname);
      } else {
        // We're presenting this group award as the supergroup award.  We want
        // the key to differ from the group award key to ensure that any award
        // kiosks update.
        $key = "speed-$place";
        $awardname = nth_fastest_in_pack($place);
      }
    } else {
      $awardname = nth_fastest_in_pack($place);
    }

    foreach ($awards as $award) {
      if ($place == $award['place']) {
        if (isset($rankid)) {
          $match = $award['rankid'] == $rankid;
        } else if (isset($classid)) {
          $match = $award['classid'] == $classid;
        } else {
          $match = true;
        }
        if ($match) {
          if ($place_index == 0) {
            $racerid = $award['racerid'];
            break;
          }
          --$place_index;
        }
      }
    }

    if ($racerid > 0) {
      $stmt = $db->prepare('SELECT racerid, :awardname AS awardname, firstname, lastname,'
                           .' carnumber, carname, class, rank, imagefile, carphoto'
                           .' FROM '.inner_join('RegistrationInfo',
                                                'Classes', 'Classes.classid = RegistrationInfo.classid',
                                                'Ranks', 'Ranks.rankid = RegistrationInfo.rankid')
                           .' WHERE racerid = :racerid');
      $stmt->execute(array(':racerid' => $racerid,
                           ':awardname' => $awardname));
    }
  } else {
    json_failure('unrecognized-award-code', "Unrecognized award code");
  }
}

if (isset($stmt)) {
  foreach ($stmt as $row) {
    $award = array('key' => $key,
                   'awardname' => $row['awardname'],
                   'reveal' => $reveal ? true : false,
                   'carnumber' => $row['carnumber'],
                   'carname' => $row['carname'],
                   'recipient' => mangled_name($row, $name_style));
    if ($use_subgroups) {
      $award['subgroup'] = @$row['rank'];
    }
    if ($row['imagefile']) {
      $award['headphoto'] = headshots()->url_for_racer($row, RENDER_CROPPED);
    }
    if ($row['carphoto']) {
      $award['carphoto'] = car_photo_repository()->url_for_racer($row, RENDER_CROPPED);
    }
    $award['duration'] = microtime(true) - $_SERVER['REQUEST_TIME_FLOAT'];
    json_out('current', $award);
    break;
  }
}

?>
