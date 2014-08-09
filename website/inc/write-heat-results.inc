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
        // ResultID, ClassID, RoundID, Heat, Lane, RacerID, ChartNumber,
        // FinishTime, FinishPlace, Points, Completed, IgnoreTime, MasterHeat
        // TODO: Beware of overwriting results!!!
        // TODO: Results from masked-out lanes are a bad sign

        // General strategy should be: reject results, but log
        // them somewhere that can be presented on coordinator
        // dashboard.
        $all_times = array();  // after sorting, place (0-based) => finishtime
        $stmt = $db->prepare('SELECT resultid, lane, finishtime FROM RaceChart'
                             .' WHERE roundid = :roundid AND heat = :heat'
                             .' ORDER BY lane');
        $stmt->execute(array(':roundid' => $roundid,
                             ':heat' => $current['heat']));
        foreach ($stmt as $row) {
          $lane = $row['lane'];
          $all_times[] = array('resultid' => $row['resultid'],
                               'lane' => $row['lane'],
                               'finishtime' => $row['finishtime']);
          if (isset($_POST['lane'.$lane])) {
            $all_times[count($all_times) - 1]['finishtime'] = $_POST['lane'.$lane];
          } else {
            echo "<missing-data lane=\"".$lane."\"/>\n";
          }
        }
        $stmt->closeCursor();

        usort($all_times, 'compare_by_finishtime');  // Now index gives finish place - 1

        foreach ($all_times as $place => $content) {
          take_action_silently('UPDATE RaceChart'
                               .' SET finishtime = :time,'
                               .' finishplace = :place,'
                               .' completed = '.db_timestamp_expr()
                               .' WHERE resultid = :resultid',
                               array(':time' => $content['finishtime'],
                                     ':place' => 1 + $place,
                                     ':resultid' => $content['resultid']));
        }

        return true;
    }
}
?>