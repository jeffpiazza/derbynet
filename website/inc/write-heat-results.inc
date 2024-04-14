<?php

require_once('inc/last-heat.inc');
require_once('inc/timer-test.inc');

function compare_by_finishtime(&$lhs, &$rhs) {
  if ($lhs['finishtime'] == $rhs['finishtime']) return 0;
  return ($lhs['finishtime'] < $rhs['finishtime']) ? -1 : 1;
}

// $_POST['lane1'], $_POST['lane2'], etc. for times
// Optionally, $_POST['place1'], $_POST['place2'], etc., for places (by lane)
//
// Returns zero-based array of {time, place}, e.g., with lane 1 result at index 0.
// Lanes without reported results appear as empty array values.
function parse_heat_results($nlanes) {
  $results = array();
  for ($i = 0; $i < $nlanes; ++$i) {
    $results[] = array();
  }

  foreach ($_POST as $key => $value) {
    if (substr($key, 0, 4) == 'lane') {
      $lane = intval(substr($key, 4));
      if (0 < $lane && $lane <= $nlanes) {
        $results[$lane - 1]['time'] = $value;
      }
    } else if (substr($key, 0, 5) == 'place') {
      $lane = intval(substr($key, 5));
      if (0 < $lane && $lane <= $nlanes) {
        $results[$lane - 1]['place'] = $value;
      }
    }
  }
  return $results;
}

// $results argument is a zero-based array of {time, place} entries, as produced by
// parse_heat_results, above.
function write_heat_results($results) {
  global $db;

  $current = get_running_round();
  $roundid = @$current['roundid'];
  $heat = @$current['heat'];

  if ($roundid == TIMER_TEST_ROUNDID) {
    // For timer testing, store (only) the most recently reported times and
    // places, in RaceInfo table, along with the "heat" with which they're
    // associated.  (tt-results-heat lets the timer page know when to clear old
    // results because new results are available.)
    timer_test_clear_old_results();
    timer_test_record_heat($heat);
    foreach ($results as $lane0 => $value) {
      if (isset($value['time'])) {
        timer_test_store_result('lane'.($lane0 + 1), $value['time']);
      }
      if (isset($value['place'])) {
        timer_test_store_result('place'.($lane0 + 1), $value['place']);
      }
    }
    return true;
  } else if (!$roundid || $roundid < 0) {
    echo '<failure code="nocurrentround">No currently-racing round is available!</failure>';
    return false;
  } else {
    // TODO: Beware of overwriting results!!!
    // TODO: Results from masked-out lanes are a bad sign
    // General strategy for problems should be: reject results, but log them
    // somewhere that can be presented on coordinator dashboard.

    if ((isset($_POST['roundid']) && $_POST['roundid'] != $roundid && $_POST['roundid'] != 0) ||
        (isset($_POST['heat']) && $_POST['heat'] != $heat && $_POST['heat'] != 0)) {
      echo "<failure code='wrong-heat'>Expecting heat results for roundid $roundid and heat $heat</failure>";
      return false;
    }

    $all_times = array();  // array of {resultid, lane, finishtime, finishplace} for scheduled lanes
    $all_times_posted = true;
    $all_places_posted = true;

    $stmt = $db->prepare('SELECT resultid, lane, finishtime, finishplace, completed FROM RaceChart'
                         .' WHERE roundid = :roundid AND heat = :heat'
                         .' ORDER BY lane');
    $stmt->execute(array(':roundid' => $roundid,
                         ':heat' => $heat));

    // For each scheduled lane, fetch existing results and overwrite new posted
    // results.
    foreach ($stmt as $row) {
      $lane = $row['lane'];
      $all_times[] = array('resultid' => $row['resultid'],
                           'lane' => $row['lane'],
                           'finishtime' => $row['finishtime'],
                           'finishplace' => $row['finishplace']);
      // If $_POST doesn't include a new value for finishtime, then leave
      // the old one (if there was one).
      if (isset($results[$lane - 1]['time']) && !empty($results[$lane - 1]['time'])) {
        $result = $results[$lane - 1]['time'];
        if ($result == '') $result = 0.0;
        $all_times[count($all_times) - 1]['finishtime'] = $result;
      } else {
        $all_times_posted = false;
      }
      if (isset($results[$lane - 1]['place']) && !empty($results[$lane - 1]['place'])) {
        $all_times[count($all_times) - 1]['finishplace'] = $results[$lane - 1]['place'];
      } else {
        $all_places_posted = false;
      }
    }
    $stmt->closeCursor();

    if (!$all_places_posted && $all_times_posted) {
      // Make up finish places based on finish times, if finish places weren't
      // included in the post.  The $all_times_posted test is to avoid computing
      // places if not all times were reported.
      usort($all_times, 'compare_by_finishtime');  // Now, index gives finish place - 1, except for ties

      $ntimes = count($all_times);
      $place = 1;
      for ($p = 0; $p < $ntimes; ++$p) {
        if (!isset($all_times[$p]['finishtime'])) {
          // $all_times_posted implies this shouldn't be possible.
          echo "\n<broken>\n";
          continue;
        }
        $all_times[$p]['finishplace'] = $place;
        if ($p + 1 < $ntimes && $all_times[$p]['finishtime'] == $all_times[$p + 1]['finishtime']) {
          // Tie; don't increment $place
        } else {
          $place = $p + 2;
        }
      }
    }

    foreach ($all_times as $content) {
      take_action_silently('UPDATE RaceChart'
                           .' SET finishtime = :time,'
                           .' finishplace = :place,'
                           .' completed = '.dbnow_expr()
                           .' WHERE resultid = :resultid',
                           array(':time' => $content['finishtime'],
                                 ':place' => $content['finishplace'],
                                 ':resultid' => $content['resultid']));
    }

    record_last_heat($roundid, $heat);
    
    return true;
  }
}
?>
