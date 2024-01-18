<?php

require_once('inc/data.inc');
require_once('inc/events.inc');

// $results, from parse_heat_results, is an array of {time, place}
// $current is null or from get_current_round, {roundid, heat}
function record_timer_finish($action, $results, $current) {
  global $db;
  return;

  $finishid = 1 + read_single_value('SELECT COALESCE(MAX(finishid), 0) FROM EventLaneDetails');
  
  $stmt = $db->prepare('INSERT INTO EventLaneDetails(finishid, lane, finishtime, finishplace)'
                       .' VALUES(:finishid, :lane, :finishtime, :finishplace)');
  $lane = 1;
  foreach($results as $result) {
    try {
      if (!$stmt->execute(array(':finishid' => $finishid,
                                ':lane' => $lane,
                                ':finishtime' => @$result['time'],
                                ':finishplace' => @$result['place']
                            ))) {
        // echo "<exec-fail>lane $lane</exec-fail>\n";
      }
    } catch (PDOException $p) {
      // echo "<fail>$p</fail>\n";
    }
    ++$lane;
  }

  record_event_sql('INSERT INTO Events(tstamp, action, roundid, heat, finishid)'
                   .' VALUES('.dbnow_expr().', :action, :roundid, :heat, :finishid)',
                   array(':action' => $action,
                         ':roundid' => @$current['roundid'],
                         ':heat' => @$current['heat'],
                         ':finishid' => $finishid));
}


function find_last_finishid_for_heat($roundid, $heat) {
  // list($action, $finishid) =
  return
     read_single_row('SELECT action, finishid FROM Events'
                     .' WHERE roundid = :roundid AND heat = :heat'
                     .'  AND action IN ('
                     .EVENT_HEAT_RESULT_FROM_TIMER.', '
                     .EVENT_HEAT_MANUALLY_ENTERED.', '
                     .EVENT_HEAT_RESULT_DISCARDED.', '
                     .EVENT_HEAT_RESULT_REINSTATED.')'
                     .' ORDER BY seq DESC',
                     array(':roundid' => $roundid,
                           ':heat' => $heat));
}

function record_event_for_discarded_heat($roundid, $heat) {
  return;
  list($action, $finishid) = find_last_finishid_for_heat($roundid, $heat);
  if ($action == EVENT_HEAT_RESULT_FROM_TIMER ||
      $action == EVENT_HEAT_MANUALLY_ENTERED ||
      $action == EVENT_HEAT_RESULT_REINSTATED) {
    record_event_sql('INSERT INTO Events(tstamp, action, roundid, heat, finishid)'
                     .' VALUES('.dbnow_expr().', :action, :roundid, :heat, :finishid)',
                     array(':action' => EVENT_HEAT_RESULT_DISCARDED,
                           ':roundid' => $roundid,
                           ':heat' => $heat,
                           ':finishid' => $finishid));
  }
  return $finishid;
}

function record_event_for_reinstated_heat($roundid, $heat) {
  return;
  list($action, $finishid) = find_last_finishid_for_heat($roundid, $heat);
  if ($action == EVENT_HEAT_RESULT_DISCARDED) {
    record_event_sql('INSERT INTO Events(tstamp, action, roundid, heat, finishid)'
                     .' VALUES('.dbnow_expr().', :action, :roundid, :heat, :finishid)',
                     array(':action' => EVENT_HEAT_RESULT_REINSTATED,
                           ':roundid' => $roundid,
                           ':heat' => $heat,
                           ':finishid' => $finishid));
  }
}

?>
