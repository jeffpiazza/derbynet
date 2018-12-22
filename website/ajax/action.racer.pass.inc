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
      not_authorized();
      return false;
    }
  } else {
    // Force reload of the page to show schedule data
    echo $failure;
    echo '<reload/>';
  }
}


start_response();

if ($racerid == 'all') {
  echo "<value>".$value."</value>\n";  // TODO
  $maxid = read_single_value('SELECT MAX(racerid) FROM RegistrationInfo');
  $ok_so_far = true;
  for ($i = 1; $i <= $maxid && $ok_so_far; ++$i) {
    $ok_so_far = pass_racer($i, $value);
  }
  if ($ok_so_far) {
    echo "<success/>\n";
  }
  echo "<reload/>\n";
} else {
  if (pass_racer($racerid, $value)) {
    echo "<success/>\n";
  }
}

end_response();

?>