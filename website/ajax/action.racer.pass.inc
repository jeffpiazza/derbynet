<?php

require_once('inc/events.inc');

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
    $failure = '<failure code="notunique">Need to identify racer uniquely.</failure>';
  }

// Protect against changes to a racer who already has schedule data.
  if ($ok_to_change) {
    if (0 + read_single_value('SELECT COUNT(*) FROM RaceChart'
                              .' WHERE roundid IN (SELECT DISTINCT roundid FROM RaceChart'
                              .'                   WHERE racerid = :racerid)'
                              .' AND (finishtime IS NOT NULL OR finishplace IS NOT NULL)',
                              array(':racerid' => $racerid), 0)) {
      $ok_to_change = false;
      $failure = '<failure code="alreadyscheduled">Schedule data already exists for this racer</failure>';
    }
  }

  if ($ok_to_change) {
    if ($value || have_permission(REVERT_CHECK_IN_PERMISSION)) {
      take_action_silently('UPDATE RegistrationInfo SET PassedInspection = :value'
                           .' WHERE RacerID = :racer',
                           array(':value' => $value,
                                 ':racer' => $racerid));
      record_event(EVENT_CHECKIN, $racerid);
      return true;
    } else {
      not_authorized_body();
      return false;
    }
  } else {
    // Force reload of the page to show schedule data
    echo $failure;
    echo '<reload/>';
  }
}


start_response();

if (pass_racer($racerid, $value)) {
  echo "<success/>\n";
}

end_response();

?>