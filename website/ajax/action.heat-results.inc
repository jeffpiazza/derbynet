<?php
// NOTE: Timer results are handled in action.timer-message.inc.  This
// script is for manually-entered results and test purposes.

// $_POST['lane1'], $_POST['lane2'], etc. for times
// Optionally, $_POST['place1'], $_POST['place2'], etc., for places (by lane)

require_once('inc/write-heat-results.inc');
require_once('inc/autoadvance.inc');

start_response();

if (have_permission(CONTROL_RACE_PERMISSION)) {
    if (write_heat_results()) {
		$current = get_running_round();
		advance_heat($current, @$current['roundid'], 'auto', false);

		echo '<success/>';
        require('ajax/query.coordinator-poll.inc');
	}
} else {
	not_authorized_body();
}

end_response();
