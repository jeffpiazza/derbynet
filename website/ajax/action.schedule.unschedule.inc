<?php
// $_POST: roundid

$roundid = $_POST['roundid'];

require_once('inc/data.inc');
require_once('inc/masterheat.inc');
$now_running = get_running_round();  // now-racing, classid, roundid, round
require_once('inc/racing-state.inc');

start_response();

if (have_permission(CONTROL_RACE_PERMISSION)) {
  $stmt = $db->prepare('DELETE FROM RaceChart WHERE roundid = :roundid');
  $stmt->execute(array(':roundid' => $roundid));

  if ($roundid == $now_running['roundid']) {
    set_racing_state(0);
    write_raceinfo('RoundID', -1);
    write_raceinfo('ClassID', -1);
    write_raceinfo('Heat', -1);
  }
  
  echo "<success/>\n";
  require('ajax/query.poll.coordinator.inc');
} else {
  not_authorized_body();
}

end_response();
?>

  
