<?php

$racerid = $_POST['racer'];

// racerid foreign key from:
// Roster
// RaceChart

start_response();

$ok = true;
if (!have_permission(EDIT_RACER_PERMISSION)) {
  not_authorized_body();
  $ok = false;
}

if ($ok &&
    read_single_value('SELECT COUNT(*) FROM RaceChart WHERE racerid = :racerid',
                      array(':racerid' => $racerid)) > 0) {
  echo "<failure code='scheduled'>Racer already appears in a race schedule.</failure>\n";
  $ok = false;
}

// It's probably not possible for a racer to get into a second roster if they
// don't have any race schedule entries, but we check anyway
if ($ok &&
    read_single_value('SELECT COUNT(*) FROM Roster WHERE racerid = :racerid',
                      array(':racerid' => $racerid)) > 1) {
  echo "<failure code='rosters'>Racer already appears in more than one racing round.</failure>\n";
  $ok = false;
}

if ($ok) {
  $ok = take_action_silently('DELETE FROM Roster WHERE racerid = :racerid',
                       array(':racerid' => $racerid));
}

if ($ok) {
  $ok = take_action_silently('DELETE FROM RegistrationInfo WHERE racerid = :racerid',
                       array(':racerid' => $racerid));
}

if ($ok) {
  echo "<success/>\n";
  echo "<reload/>\n";
}

end_response();

?>