<?php
// $_POST['roundid']

// TODO: Modifying charts with BYEs.
// TODO: Permissions

// Returns a two-dimensional array, with $contests[i][j] giving the
// number of times car i faces car j in a heat, for i != j.
function contests(&$heats, $ncars) {
	$contests = array_fill(0, $ncars, null);
	for ($car = 0; $car < $ncars; ++$car) {
		$contests[$car] = array_fill(0, $ncars, 0);
	}

	foreach ($heats as $heat) {
		foreach ($heat as $car1) {
			foreach ($heat as $car2) {
				if ($car1 != $car2 && $car1 !== null && $car2 !== null) {
					++$contests[$car1][$car2];
				}
			}
		}
	}

	// echo '<h2>Contest matrix:</h2>'."\n"; print_contests($contests);

	return $contests;
}

// TODO: Don't pass $ncars, calculate a concentrator instead
// TODO: $n_times_per_lane
function exhaustive_late_checkin($heats, $ncars, $ncommitted) {
	$new_racer = $ncars;
	$nlanes = count($heats[0]);
	// $n_times_per_lane = count($heats) /$ncars
	$new_heat = array_fill(0, $nlanes, null);
	$heats[] = &$new_heat;
	$new_heat[0] = $new_racer;
	$best_score = array('score' => 10000,
						'choices' => null);
	exhaustive_lane_checkin_recur($heats, $ncars, $ncommitted, $new_racer, array(), /* lane */ 1, $best_score);

	foreach ($best_score['choices'] as $lane => $heat_index) {
		$new_heat[$lane] = $heats[$heat_index][$lane];
		$heats[$heat_index][$lane] = $new_racer;
	}

	return $heats;
}

function exhaustive_lane_checkin_recur(&$heats, $ncars, $ncommitted, $new_racer, $choices, $lane, &$best_score) {
	$nlanes = count($heats[0]);
	if ($lane >= $nlanes) {
		$score = score_heats($heats);
		// echo '<pre>';print_r($choices);echo '</pre>'; print_chart($heats); print_heat_score($heats);
		if ($score < $best_score['score'] - 0.00001) {
			$best_score['score'] = $score;
			$best_score['choices'] = $choices;  // Copies $choices
		}
	} else {
		$new_heat = &$heats[count($heats) - 1];
		for ($heat_index = $ncommitted; $heat_index < count($heats) - 1; ++$heat_index) {
			$candidate = $heats[$heat_index][$lane];

			$allowable = true;
			// Ensure that $new_racer isn't already in this heat
			for ($i = 0; $i < $nlanes; ++$i) {
				if ($heats[$heat_index][$i] == $new_racer || $new_heat[$i] == $candidate) {
					$allowable = false;
				}
			}
			if (!$allowable) {
				continue;
			}

			$heats[$heat_index][$lane] = $new_racer;
			$new_heat[$lane] = $candidate;
			$choices[$lane] = $heat_index;

			exhaustive_lane_checkin_recur($heats, $ncars, $ncommitted, $new_racer, $choices, $lane + 1, $best_score);

			// Put it back
			$heats[$heat_index][$lane] = $candidate;
		}
	}
}

// A perfect schedule should have a score of 0.
// A partial perfect schedule should have a score of $ncars * ($ncars - 1) / 2 or less.

function score_heats(&$heats, $n_times_per_lane = 1) {
	$nlanes = count($heats[0]);

	$inv_roster = construct_inverted_roster($heats);
	$ncars = count($inv_roster);

	$goal = $n_times_per_lane * $nlanes * ($nlanes - 1) / ($ncars - 1);
	$contests = contests($heats, $ncars);
	//print_contests($contests);

	$score = 0;
	for ($r0 = 0; $r0 < count($contests); ++$r0) {
		for ($r1 = 0; $r1 < count($contests[$r0]); ++$r1) {
			$c = $contests[$r0][$r1];
			if ($r0 != $r1 && $c !== null) {
				$score += ($goal - $c) * ($goal - $c);
			}
		}
	}

	return $score;
}

function print_heat_score(&$heats, $n_times_per_lane = 1) {
	$s = score_heats($heats);
	$nlanes = count($heats[0]);

	$inv_roster = construct_inverted_roster($heats);
	$ncars = count($inv_roster);

	print_contests(contests($heats, $ncars));

	$goal = $n_times_per_lane * $nlanes * ($nlanes - 1) / ($ncars - 1);
	$limit = $ncars * ($ncars - 1) / 2;
	echo '<p>Deviation of '.$s.' from a goal of '.$goal.': ';
	if ($s == 0) {
		echo 'PERFECT!';
	} else if ($s <= $limit) {
		echo 'partial perfect (limit '.$limit.')';
	} else {
		echo 'nothing special (limit '.$limit.')';
	}
	echo '</p>'."\n";
}


// TODO: Remove this (for debugging only)
function print_chart(&$heats) {
	echo '<table>';
	$first = true;
	foreach ($heats as $heat) {
		if ($first) {
			echo '<tr>';
			for ($lane = 1; $lane <= count($heat); ++$lane) {
				echo '<th>Lane '.$lane.'</th>';
			}
			echo '</tr>'."\n";
			$first = false;
		}
		echo '<tr>';
		foreach ($heat as $car) {
			echo '<td>'.$car.'</td>';
		}
		echo '</tr>'."\n";
	}
	echo '</table>'."\n";
}

function print_contests(&$contests) {
	echo '<table>';
	$heati = 0;
	foreach ($contests as $heat) {
		if ($heati == 0) {
			echo '<tr><th/>';
			for ($lane = 0; $lane < count($heat); ++$lane) {
				echo '<th>R'.$lane.'</th>';
			}
			echo '</tr>'."\n";
		}
		echo '<tr><th>R'.$heati.'</th>';
		++$heati;
		foreach ($heat as $car) {
			echo '<td>'.$car.'</td>';
		}
		echo '</tr>'."\n";
	}
	echo '</table>'."\n";
}

function heat_as_string($heat) {
	$s = "[";
	$first = true;
	foreach ($heat as $racer) {
		if ($first) {
			$first = false;
		} else {
			$s .= ",";
		}
		$s .= $racer;
	}
	return $s."]";
}

function read_race_chart($roundid) {
	global $db;
	$heats = array();
	$stmt = $db->prepare('SELECT heat, lane, racerid FROM RaceChart WHERE roundid = :roundid ORDER BY heat, lane');
	$stmt->execute(array(':roundid' => $roundid));
	foreach ($stmt as $row) {
		while ($row['heat'] > count($heats)) {
			$heats[] = array();
		}
		while ($row['lane'] > count($heats[$row['heat'] - 1])) {
			$heats[$row['heat'] - 1][] = 'Bye';
		}
		$heats[$row['heat'] - 1][$row['lane'] - 1] = $row['racerid'];
	}

	return $heats;
}

// roster[sequence] = racerid
function read_roster($roundid) {
	global $db;
	$roster = array();
	$stmt = $db->prepare('SELECT rosterid, Roster.racerid FROM Roster'
						 .' INNER JOIN RegistrationInfo'
						 .' ON Roster.racerid = RegistrationInfo.racerid'
						 .' WHERE roundid = :roundid'
						 .' AND passedinspection = 1'
						 .' ORDER BY rosterid');
	$stmt->execute(array(':roundid' => $roundid));
	foreach ($stmt as $row) {
		$roster[] = $row['racerid'];
	}
	return $roster;
}

function form_chart(&$schedule, &$roster) {
	$heats = array();
	foreach ($schedule as $sched_heat) {
		$heat = array();
		foreach ($sched_heat as $index) {
			$heat[] = $roster[$index];
		}
		$heats[] = $heat;
	}
	return $heats;
}

function invert_roster(&$roster) {
	$inverted = array();
	foreach ($roster as $seq => $racerid) {
		$inverted[$racerid] = $seq;
	}
	return $inverted;
}

function construct_inverted_roster(&$heats) {
	$inv_roster = array();  // racerid => 0-based index
	foreach ($heats as $heat) {
		foreach ($heat as $racerid) {
			if (!array_key_exists($racerid, $inv_roster)) {
				$inv_roster[$racerid] = count($inv_roster);
			}
		}
	}
	return $inv_roster;
}


function rewrite_chart($roundid, $ncommitted, &$heats) {
	global $db;

	$db->exec('DELETE FROM RaceChart WHERE roundid = '.$roundid
			  .' AND heat > '.$ncommitted  // 1-based heats here
			  .' AND finishtime IS NULL AND finishplace IS NULL');

	$stmt = $db->prepare('INSERT INTO RaceChart(roundid, heat, lane, racerid)'
						 .' VALUES(:roundid, :heat, :lane, :racerid)');
	for ($heat_index = $ncommitted; $heat_index < count($heats); ++$heat_index) {
		$heat = &$heats[$heat_index];
		$lane_index = 0;
		foreach ($heat as $racerid) {
			++$lane_index;
			if ($racerid) {
				$stmt->execute(array(':roundid' => $roundid,
									 ':heat' => $heat_index + 1,
									 ':lane' => $lane_index,
									 ':racerid' => $racerid));
			}
		}
	}
}

start_response();

if (have_permission(CONTROL_RACE_PERMISSION)) {
  $roundid = $_POST['roundid'];

  $chart = read_race_chart($roundid);

  $inv_roster = construct_inverted_roster($chart);
  $ncars = count($inv_roster);

  $heats = form_chart($chart, $inv_roster);

  $ncommitted = read_single_value('SELECT MAX(heat) FROM RaceChart'
                                  .' WHERE roundid = :roundid'
                                  .' AND (finishtime IS NOT NULL OR finishplace IS NOT NULL)',
                                  array(':roundid' => $roundid),
                                  0);

  if ($ncommitted == 0) {
	echo '<failure code="notapplicable">No race results exist for this round; generate normal schedule.</failure>';
  } else {

    // Seems like the existence of the roster record probably implies that
    // the racer passed inspection, but it should be harmless to verify
    // this way, too.
	$new_racer = read_single_value('SELECT MIN(racerid) FROM Roster'
								   .' WHERE roundid = :roundid'
								   .' AND EXISTS(SELECT 1 FROM RegistrationInfo'
                                   .' WHERE racerid = Roster.racerid'
                                   .' AND passedinspection = 1)'
	    						   .' AND NOT EXISTS(SELECT 1 FROM RaceChart'
                                   .' WHERE roundid = Roster.roundid'
                                   .' AND RaceChart.racerid = Roster.racerid)',
				    			   array(':roundid' => $roundid));

	if (!$new_racer) {
      echo '<failure code="noracer">No new racer appears in the roster that\'s not also in the schedule.</failure>';
	} else {
      $new_heats = exhaustive_late_checkin($heats, $ncars, $ncommitted);
      //print_chart($new_heats);
      //print_heat_score($new_heats);

      if (!$new_heats) {
        echo '<failure code="unable">Unsuccessful reschedule attempt.</failure>';
      } else {
        $roster = invert_roster($inv_roster);
        $roster[] = $new_racer;
        $chart = form_chart($new_heats, $roster);
        rewrite_chart($roundid, $ncommitted, $chart);
        echo '<success/>';
      }
	}
  }
} else {
  not_authorized_body();
}
end_response();
?>
