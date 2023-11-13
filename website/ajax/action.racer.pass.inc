<?php

require_once('inc/events.inc');
require_once('inc/schema_version.inc');

$value = isset($_POST['value']) ? $_POST['value'] : 1;
$racerid = @$_POST['racer'];
if (isset($_POST['barcode'])) {
  require_once('inc/barcode.inc');
  $racerid = barcode_to_racerid($_POST['barcode']);
}

function pass_racer($racerid, $value) {
  $ok_to_change = true;

  if ($racerid == 0) {
    $ok_to_change = false;
    json_failure('notunique', "Need to identify racer uniquely.");
  }

// Protect against changes to a racer who already has schedule data.
  if ($ok_to_change) {
    if (0 + read_single_value('SELECT COUNT(*) FROM RaceChart'
                              .' WHERE roundid IN (SELECT DISTINCT roundid FROM RaceChart'
                              .'                   WHERE racerid = :racerid)'
                              .' AND (finishtime IS NOT NULL OR finishplace IS NOT NULL)',
                              array(':racerid' => $racerid), 0)) {
      $ok_to_change = false;
      json_failure('alreadyscheduled', "Schedule data already exists for this racer");
    }
  }

  if ($ok_to_change) {
    if ($value || have_permission(REVERT_CHECK_IN_PERMISSION)) {
      take_action_silently('UPDATE RegistrationInfo'
                           .' SET PassedInspection = :value'
                           .($value && schema_version() < REGISTRATION_CHECKIN_SCHEMA ? ''
                             : ',    checkin_time = '.floor(microtime(true) * 1000))
                           .' WHERE RacerID = :racer',
                           array(':value' => $value,
                                 ':racer' => $racerid));
      record_event(EVENT_CHECKIN, array('racerid' => $racerid));
      return true;
    } else {
      json_not_authorized();
      return false;
    }
  } else {
    json_out('reload', true);
  }
}


if (pass_racer($racerid, $value)) {
  json_success();
  if ($value) {
    $details = read_single_row('SELECT class, firstname, lastname'
                               .' FROM RegistrationInfo'
                               .' INNER JOIN Classes ON Classes.classid = RegistrationInfo.classid'
                               .' WHERE racerid = :racerid',
                               array(':racerid' => $racerid),
                               PDO::FETCH_ASSOC);
    if ($details) {
      json_out('details', $details);
    }
  }
}

?>
