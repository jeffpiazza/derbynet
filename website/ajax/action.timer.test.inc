<?php

// $_POST['tt-mask']
require_once('inc/timer-test.inc');

if (isset($_POST['tt-mask'])) {
  timer_test_update_lane_mask($_POST['tt-mask']);
  // Trigger a new heat, so the timer picks up the new mask
  timer_test_start();
  json_success();
} else if (isset($_POST['test-mode'])) {
  if ($_POST['test-mode']) {
    timer_test_start();
  } else {
    timer_test_end();
  }
  json_success();
} else {
  json_failure('unknown-operation', "");
}

?>