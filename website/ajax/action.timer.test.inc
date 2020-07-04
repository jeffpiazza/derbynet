<?php

// $_POST['tt-mask']

start_response();

function start_timer_testing() {
  $current = get_running_round();
  if ($current['roundid'] >= 0) {
    write_raceinfo('tt-roundid', $current['roundid']);
    write_raceinfo('tt-heat', $current['heat']);
  }
  write_raceinfo('RoundID', TIMER_TEST_ROUNDID);
  write_raceinfo('Heat', (isset($current['heat']) && $current['heat'] > 0)
                 ? $current['heat'] + 1
                 : 1);
  write_raceinfo('NowRacingState', 1);
}

function end_timer_testing() {
  if (read_raceinfo('RoundID') == TIMER_TEST_ROUNDID) {
    write_raceinfo('RoundID', read_raceinfo('tt-roundid', -1));
    write_raceinfo('Heat', read_raceinfo('tt-heat', -1));
    write_raceinfo('NowRacingState', 0);
  }
}

function update_lane_mask($mask) {
  end_timer_testing();
  write_raceinfo('tt-mask', $mask);
}


if (isset($POST['tt-mask'])) {
  update_lane_mask($_POST['tt-mask']);
  echo "<success/>\n";
} else if (isset($_POST['test-mode'])) {
  if ($_POST['test-mode']) {
    start_timer_testing();
  } else {
    end_timer_testing();
  }
  echo "<success/>\n";
} else {
  echo "<failure code='unknown-operation'></failure>\n";
}

end_response();
?>