<?php
if ($_POST['previous']) {
	// TODO: Something is writing NULLs into ImageFile field.  Does
	// Access treat an empty string as NULL?
	take_action_silently('UPDATE RegistrationInfo SET imagefile = \'\''
						 .' WHERE racerid = :racerid',
						 array(':racerid' => $_POST['previous']));
}

take_action('UPDATE RegistrationInfo SET imagefile = :photo'
			.' WHERE racerid = :racerid',
			array(':photo' => $_POST['photo'],
				  ':racerid' => $_POST['racer']));
?>