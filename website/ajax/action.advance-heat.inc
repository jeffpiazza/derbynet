<?php
// $_POST['roundid'] (defaults to current one)
// $_POST['heat'] (values may be numeric, 'auto', 'next', or 'prev', defaults to 'auto')

require_once('inc/autoadvance.inc');

start_response();

if (have_permission(CONTROL_RACE_PERMISSION)) {
	$current = get_running_round();

    // TODO: No current roundid?
	$roundid = @$current['roundid'];
	if (isset($_POST['roundid'])) {
		$roundid = $_POST['roundid'];
	}
	$heat = 'auto';
	if (isset($_POST['heat'])) {
		$heat = $_POST['heat'];
	}


	if (advance_heat($current, $roundid, $heat, true)) {
		echo '<success/>';
	}
} else {
	not_authorized_body();
}

end_response();
?>
