<?php
if (have_permission(ASSIGN_RACER_IMAGE_PERMISSION) && isset($_POST['previous'])) {
	// TODO: Something is writing NULLs into ImageFile field.  Does
	// Access treat an empty string as NULL?
	take_action_silently('UPDATE RegistrationInfo SET imagefile = \'\''
						 .' WHERE racerid = :racerid',
						 array(':racerid' => $_POST['previous']));
}

take_action_if_permitted('UPDATE RegistrationInfo SET imagefile = :photo'
                         .' WHERE racerid = :racerid',
                         array(':photo' => $_POST['photo'],
                               ':racerid' => $_POST['racer']),
                         ASSIGN_RACER_IMAGE_PERMISSION);
?>