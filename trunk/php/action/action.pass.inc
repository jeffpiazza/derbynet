<?php
$ok_to_change = true;

// Protect against changes to a racer who already has schedule data.
if (0 + read_single_value('SELECT COUNT(*) FROM RaceChart WHERE RacerID = '.$_POST['racer'], 0))
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
	echo '<failure>Schedule data already exists for this racer</failure><reload/>';
	end_response();
}

?>