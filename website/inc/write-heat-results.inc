<?php


function compare_by_finishtime(&$lhs, &$rhs) {
  if ($lhs['finishtime'] == $rhs['finishtime']) return 0;
  return ($lhs['finishtime'] < $rhs['finishtime']) ? -1 : 1;
}

// $_POST['lane1'], $_POST['lane2'], etc. for times
// Optionally, $_POST['place1'], $_POST['place2'], etc., for places (by lane)

function write_heat_results() {
  global $db;

  $current = get_running_round();
  $roundid = @$current['roundid'];

  if (!$roundid || $roundid < 0) {
    echo '<failure code="nocurrentround">No currently-racing round is available!</failure>';
    return false;
  } else {
    // TODO: Beware of overwriting results!!!
    // TODO: Results from masked-out lanes are a bad sign
    // General strategy for problems should be: reject results, but log them
    // somewhere that can be presented on coordinator dashboard.

    $all_times = array();  // array of {resultid, lane, finishtime, finishplace} for scheduled lanes
    $stmt = $db->prepare('SELECT resultid, lane, finishtime, finishplace, completed FROM RaceChart'
                         .' WHERE roundid = :roundid AND heat = :heat'
                         .' ORDER BY lane');
    $stmt->execute(array(':roundid' => $roundid,
                         ':heat' => $current['heat']));

    // For each scheduled lane, fetch existing results and overwrite new posted
    // results.
    $all_places_present = true;
    foreach ($stmt as $row) {
      $lane = $row['lane'];
      $all_times[] = array('resultid' => $row['resultid'],
                           'lane' => $row['lane'],
                           'finishtime' => $row['finishtime'],
                           'finishplace' => $row['finishplace']);
      // If $_POST doesn't include a new value for finishtime, then leave
      // the old one (if there was one).
      if (isset($_POST['lane'.$lane])) {
        $all_times[count($all_times) - 1]['finishtime'] = $_POST['lane'.$lane];
      }
      if (isset($_POST['place'.$lane])) {
        $all_times[count($all_times) - 1]['finishplace'] = $_POST['place'.$lane];
      } else {
        $all_places_present = false;
      }
    }
    $stmt->closeCursor();

    if (!$all_places_present) {
      usort($all_times, 'compare_by_finishtime');  // Now index gives finish place - 1
      for ($i = 0; $i < count($all_times); ++$i) {
        $all_times[$i]['finishplace'] = 1 + $i;
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

    return true;
  }
}
?>