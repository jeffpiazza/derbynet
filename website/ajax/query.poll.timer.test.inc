<timer-test>
<?php
require_once('inc/current-heat.inc');
require_once('inc/timer-state-xml.inc');
require_once('inc/timer-test.inc');

$now_running = get_running_round();
emit_current_heat($now_running);
emit_timer_state();
emit_timer_details();
?>

<?php

$tt = timer_test_recorded_results();

echo "<tt-results heat=\"$tt[heat]\"/>\n";
echo "<tt-mask>$tt[mask]</tt-mask>\n";

foreach($tt['results'] as $r) {
  if (isset($r['time'])) {
    echo "<tt lane='$r[lane]' time='$r[time]'/>\n";
  } else {
    echo "<tt lane='$r[lane]' place='$r[place]'/>\n";
  }
}
?>

</timer-test>
