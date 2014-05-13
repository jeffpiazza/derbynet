<?php
$ok_to_change = true;

// Protect against changes to a racer who already has schedule data.
if (0 + read_single_value('SELECT COUNT(*) FROM RaceChart'
						  .' WHERE roundid IN (SELECT DISTINCT roundid FROM RaceChart'
						                    .' WHERE racerid = :racerid)'
						  .' AND finishtime IS NOT NULL',
						  array(':racerid' => $_POST['racer']), 0))
	$ok_to_change = false;

if ($ok_to_change) {
	if ($_POST['value'] || have_permission(REVERT_CHECK_IN_PERMISSION)) {
		take_action('UPDATE RegistrationInfo SET PassedInspection = :value'
					.' WHERE RacerID = :racer',
					array(':value' => $_POST['value'],
						  ':racer' => $_POST['racer']));
	} else {
		not_authorized();
	}
} else {
	// Force reload of the page to show schedule data
	start_response();
	echo '<failure code="alreadyscheduled">Schedule data already exists for this racer</failure>';
    echo '<reload/>';
	end_response();
}

?>