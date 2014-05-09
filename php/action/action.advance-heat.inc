<?php
// $_POST['roundid'] (defaults to current one)
// $_POST['heat'] (values may be numeric, 'auto', 'next', or 'prev', defaults to 'auto')

// TODO: permissions

start_response();

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
	echo '<failure code="nosuchheat">No heat '.$heat.' scheduled for roundid '.$roundid.'</failure>';
} else {
	write_raceinfo('RoundID', $roundid);
	write_raceinfo('Heat', $heat);
	echo '<success/>';
}

end_response();
?>
