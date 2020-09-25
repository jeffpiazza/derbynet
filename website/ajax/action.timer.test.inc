<?php

// $_POST['tt-mask']
require_once('inc/timer-test.inc');

start_response();

if (isset($_POST['tt-mask'])) {
  timer_test_update_lane_mask($_POST['tt-mask']);
  echo "<success/>\n";
} else if (isset($_POST['test-mode'])) {
  if ($_POST['test-mode']) {
    timer_test_start();
  } else {
    timer_test_end();
  }
  echo "<success/>\n";
} else {
  echo "<failure code='unknown-operation'></failure>\n";
}

end_response();
?>