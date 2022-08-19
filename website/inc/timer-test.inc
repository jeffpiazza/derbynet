<?php

// When timer testing starts, the current heat, if any is stored in tt-roundid
// and tt-heat, and then restored when exiting timer testing.
//
// For timer testing, a time recorded for lane 1 is stored in RaceInfo with key
// tt-lane1.  A place is recorded as tt-place1.
//
// Timer testing always uses RoundID == TIMER_TEST_ROUNDID (= -100), but the
// heat varies for each run; the heat for the currently-stored results is
// recorded with key tt-results-heat.
//
// tt-mask is a numeric mask of unused lanes (0 means all lanes in use) for
// timer testing.

require_once('inc/data.inc');

function timer_test_start() {
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
  write_raceinfo('refused-results', 0);
}

function timer_test_end() {
  if (read_raceinfo('RoundID') == TIMER_TEST_ROUNDID) {
    write_raceinfo('RoundID', read_raceinfo('tt-roundid', -1));
    write_raceinfo('Heat', read_raceinfo('tt-heat', -1));
    write_raceinfo('NowRacingState', 0);
    return true;
  }
  return false;
}

function timer_test_update_lane_mask($mask) {
  $was_running = timer_test_end();
  write_raceinfo('tt-mask', $mask);
  if ($was_running) {
    timer_test_start();
  }
}

// $key is of the form 'lane1' or 'place1'
function timer_test_store_result($key, $value) {
  write_raceinfo('tt-'.$key, $value);
}

function timer_test_clear_old_results() {
  global $db;
  $db->exec('DELETE FROM RaceInfo WHERE itemkey LIKE \'tt-lane%\' OR itemkey LIKE \'tt-place%\'');
}

function timer_test_record_heat($heat) {
  write_raceinfo('tt-results-heat', $heat);
}

// Returns { heat: mask: results: [{lane:, time:, place:}, ...]}
function timer_test_recorded_results() {
  global $db;
  $result = array('heat' => 0,
                  'mask' => 0,
                  'results' => array());
  foreach($db->query('SELECT itemkey, itemvalue FROM RaceInfo'
                     .' WHERE itemkey LIKE \'tt-%\'') as $row) {
    if ($row['itemkey'] == 'tt-results-heat') {
      $result['heat'] = $row['itemvalue'];
    } else if ($row['itemkey'] == 'tt-mask') {
      $result['mask'] = $row['itemvalue'];
    } else if (substr($row['itemkey'], 0, 7) == 'tt-lane') {
      $result['results'][] = array('lane' => substr($row['itemkey'], 7), 'time' => $row['itemvalue']);
    } else if (substr($row['itemkey'], 0, 8) == 'tt-place') {
      $result['results'][] = array('lane' => substr($row['itemkey'], 8), 'place' => $row['itemvalue']);
    }
  }

  // Merge results for the same lane
  usort($result['results'], function($a, $b) {
      $alane = intval($a['lane']);
      $blane = intval($b['lane']);
      if ($alane < $blane) return -1;
      if ($alane > $blane) return 1;
      return 0;
    });
  $times = array();
  for ($i = 0; $i < count($result['results']); ++$i) {
    if ($i + 1 < count($result['results']) &&
        $result['results'][$i]['lane'] == $result['results'][$i + 1]['lane']) {
      if (isset($result['results'][$i + 1]['time'])) {
        $result['results'][$i]['time'] = $result['results'][$i + 1]['time'];
      }
      if (isset($result['results'][$i + 1]['place'])) {
        $result['results'][$i]['place'] = $result['results'][$i + 1]['place'];
      }
      array_splice($result['results'], $i + 1, 1);
    }
    if (isset($result['results'][$i]['time'])) {
      $time = floatval($result['results'][$i]['time']);
      $result['results'][$i]['time'] = $time;
      $times[] = $time;
    }
  }

  // Calculate place from time
  sort($times);
  for ($i = 0; $i < count($result['results']); ++$i) {
    if (!isset($result['results'][$i]['place'])) {
      $result['results'][$i]['place'] = 1 + array_search($result['results'][$i]['time'], $times);
    }
  }
  
  return $result;
}

?>
