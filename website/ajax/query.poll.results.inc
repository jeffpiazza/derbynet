 <?php
require_once('inc/json-current-heat.inc');
require_once('inc/autoadvance.inc');
require_once('inc/ordinals.inc');

require_once('inc/rounds.inc');

$now_running = get_running_round();

// $_GET['roundid']
$roundid = $now_running['roundid'];
if (isset($_GET['roundid'])) $roundid = $_GET['roundid'];
$with_details = isset($_GET['details']);  // Set for testing

json_out('schedule-signature', schedule_signature());

$use_points = read_raceinfo_boolean('use-points');
$time_format = get_finishtime_formatting_string();

$results = array();
$stmt = $db->prepare('SELECT'
                     .' resultid, Rounds.roundid, heat, lane, racerid, finishtime, finishplace, completed'
                     .' FROM Rounds'
                     .' INNER JOIN RaceChart'
                     .' ON RaceChart.roundid = Rounds.roundid'
                     .' WHERE RaceChart.roundid = :roundid'
                     .'  OR completed > '.db_compare_completed(':since'));
$stmt->execute(array(':roundid' => $roundid,
                     // Include new results from the last 5 minutes, even if not
                     // in the same round
                     ':since' => date('Y-m-d H:i:s', time() - 300)));
foreach ($stmt as $row) {
  if (!isset($row['finishtime']) && !isset($row['finishplace'])) {
    $val = '--';
  } else if ($use_points) {
    $val = ordinal($row['finishplace']);
  } else {
    $val = sprintf($time_format, $row['finishtime']);
  }
  $result = $with_details ? $row : array('resultid' => $row['resultid']);
  $result['outcome'] = $val;
  $results[] = $result;
}
json_out('results', $results);

json_out('current-heat', json_current_heat($now_running));

$next = get_next_heat($now_running, 'next');
if ($next !== false) {
  json_out('next-heat', array('roundid' => $next['roundid'],
                              'heat' => $next['heat']));
}
?>
