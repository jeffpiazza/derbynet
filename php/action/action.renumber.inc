<?php

$passed_inspection = read_single_value('SELECT PassedInspection OR CarNumber = 99999'
									   .' FROM RegistrationInfo'
									   .' WHERE RacerID = '.$_POST['racer']);

take_action_if_permitted('UPDATE RegistrationInfo'
						 .' SET PassedInspection = PassedInspection OR CarNumber = 99999,'
						 .' CarNumber = :carnumber'
						 .' WHERE RacerID = :racerid',
						 array(':carnumber' => $_POST['value'],
							   ':racerid' => $_POST['racer']),
						 RENUMBER_CAR_PERMISSION,
						 // checkin.js looks for this element, to tell
						 // it if the car was passed as a side-effect
						 // of renumbering
						 $passed_inspection ? '<passed racer="'.$_POST['racer'].'"/>' : ''
	);
?>