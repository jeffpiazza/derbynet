<just-results>
 <?php
require_once('inc/current-heat.inc');
require_once('inc/autoadvance.inc');
require_once('inc/ordinals.inc');

require_once('inc/rounds.inc');

$now_running = get_running_round();

// $_GET['roundid']
$roundid = $now_running['roundid'];
if (isset($_GET['roundid'])) $roundid = $_GET['roundid'];
$with_details = isset($_GET['details']);

write_signatures();

$use_points = read_raceinfo_boolean('use-points');
$time_format = get_finishtime_formatting_string();

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
  $details = '';
  if ($with_details) {
    $details = " roundid='".$row['roundid']."' heat='".$row['heat']."' lane='".$row['lane']."'";
    $details .= " racerid='".$row['racerid']."'";
  }

  echo "<result resultid='".$row['resultid']."'$details outcome='".$val."'/>\n";
}

emit_current_heat($now_running);

$next = get_next_heat($now_running, 'next');
if ($next !== false) {
  echo '  <next-heat roundid="'.$next['roundid'].'" heat="'.$next['heat'].'"/>'."\n";
}
?>
</just-results>