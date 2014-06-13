<?php

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
        $resultids = array();  // lane => resultid
        $all_times = array();  // after sorting, place (0-based) => finishtime
        $stmt = $db->prepare('SELECT resultid, lane FROM RaceChart'
                             .' WHERE roundid = :roundid AND heat = :heat'
                             .' ORDER BY lane');
        $stmt->execute(array(':roundid' => $roundid,
                             ':heat' => $current['heat']));
        foreach ($stmt as $row) {
            $resultids[$row['lane']] = $row['resultid'];
            $all_times[] = @$_POST['lane'.$row['lane']];
        }
        $stmt->closeCursor();

        sort($all_times);

        foreach ($resultids as $lane => $resultid) {
            $time = @$_POST['lane'.$lane];  // TODO: Missing?
            $place = @$_POST['place'.$lane];
            if ($place === null) {
                $place = 1 + array_search($time, $all_times);
            }
            take_action_silently('UPDATE RaceChart'
                                 .' SET finishtime = :time,'
                                 .' finishplace = :place,'
                                 .' completed = '.db_timestamp_expr()
                                 .' WHERE resultid = :resultid',
                                 array(':time' => $time,
                                       ':place' => $place,
                                       ':resultid' => $resultid));
        }

        return true;
    }
}
?>