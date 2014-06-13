<?php

function advance_heat(&$current, $roundid, $heat, $echo_failure) {
	$auto = $heat == 'auto';
	if ($heat == 'next' || $auto) {
		$prev_heat = isset($current['heat']) ? $current['heat'] : 1;
		$heat = $prev_heat + 1;
	} else if ($heat == 'prev') {
		$prev_heat = isset($current['heat']) ? $current['heat'] : 1;
		$heat = $prev_heat - 1;
	}

	$ok = read_single_value('SELECT COUNT(*) FROM RaceChart'
							.' WHERE roundid = :roundid AND heat = :heat',
							array(':roundid' => $roundid,
								  ':heat' => $heat)) > 0;

    // TODO: Ensure not already-completed race results?
    // TODO: This runs rounds in order of roundid, which may well not be the order for racing
	if (!$ok && $auto) {
		$r2 = read_single_value('SELECT MIN(roundid) FROM RaceChart'
								.' WHERE roundid > :roundid',
								array(':roundid' => $roundid));
		if ($r2) {
			$roundid = $r2;
			$heat = 1;
			$ok = read_single_value('SELECT COUNT(*) FROM RaceChart'
									.' WHERE roundid = :roundid AND heat = :heat',
									array(':roundid' => $roundid,
										  ':heat' => $heat)) > 0;
		}
	}


	if (!$ok) {
		if ($echo_failure) {
			echo '<failure code="nosuchheat">No heat '.$heat.' scheduled for roundid '.$roundid.'</failure>';
		}
	} else {
		write_raceinfo('RoundID', $roundid);
		write_raceinfo('Heat', $heat);
		if ($roundid != @$current['roundid']) {
			write_raceinfo('ClassID', read_single_value('SELECT classid FROM Rounds WHERE roundid = :roundid',
														array(':roundid' => $roundid)));
		}
	}

	return $ok;
}

?>