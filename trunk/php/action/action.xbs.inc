<?php
if ($_POST['value']) {
	take_action('INSERT INTO Awards(AwardName, AwardTypeID, RacerID)'
				.' VALUES(:award, 3, :racerid)',
				array(':award' => read_raceinfo('xbs-award'),
					  ':racerid' => $_POST['racer']));
} else {
	take_action('DELETE FROM Awards WHERE racerid = :racerid',
				array(':racerid' => $_POST['racer']));
}
?>