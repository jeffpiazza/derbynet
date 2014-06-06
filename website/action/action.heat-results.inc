<?php
// $_POST['lane1'], $_POST['lane2'], etc. for times
// TODO: Optionally, $_POST['place1'], $_POST['place2'], etc., for places (by lane)

// TODO: Relay completion events?

require_once('action/write-heat-results.inc');
require_once('action/autoadvance.inc');

start_response();

if (have_permission(CONTROL_RACE_PERMISSION)) {
    if (write_heat_results()) {
		$current = get_running_round();
		advance_heat($current, @$current['roundid'], 'auto', false);

		echo '<success/>';
	}
} else {
	not_authorized_body();
}

end_response();