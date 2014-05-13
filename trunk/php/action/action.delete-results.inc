<?php
// $_POST['roundid'] = number, or 'all'

start_response();

if (have_permission(CONTROL_RACE_PERMISSION)) {
	$roundid = @$current['roundid'];
	if ($roundid == 'all') {
		$db->exec('UPDATE RaceChart'
				  .' SET finishtime = NULL, finishplace = NULL, completed = NULL');
	} else {
		$stmt = $db->prepare('UPDATE RaceChart'
							 .' SET finishtime = NULL, finishplace = NULL, completed = NULL'
							 .' WHERE roundid = :roundid');
		$stmt->execute(array(':roundid' => $roundid));
		echo '<success/>';
	}
} else {
	not_authorized_body();
}

end_response();