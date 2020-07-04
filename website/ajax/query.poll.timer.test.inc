<timer-test>
<?php
require_once('inc/current-heat.inc');
require_once('inc/timer-state-xml.inc');

$now_running = get_running_round();
emit_current_heat($now_running);
emit_timer_state();
emit_timer_details();
?>

<?php
foreach($db->query('SELECT itemkey, itemvalue FROM RaceInfo'
                   .' WHERE itemkey LIKE \'tt-%\'') as $row) {
  if ($row['itemkey'] == 'tt-results-heat') {
    echo "<tt-results heat=\"$row[itemvalue]\"/>\n";
  } else {
    echo "<tt key=\"$row[itemkey]\">$row[itemvalue]</tt>\n";
  }
}
?>

</timer-test>
