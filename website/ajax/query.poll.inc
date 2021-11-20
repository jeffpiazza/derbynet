<?php
// $_GET['values']
//
// $_GET['roundid'], $_GET['heat'] for 'heat-results'
// $_GET['row-height'] for 'racers' (TODO - don't need this)

function json_common_1($key, &$now_running) {
  global $db;

  switch ($key) {
  case 'awards': /*'awards' (query.ballot.results : vote_tabulations)
                   'awards' (via json_out_awards: award.winner) */ break;
  case 'catalog-entry': /* standings.reveal */ break;
  case 'best-times':
    // TODO Needs refinement
    $stmt = $db->prepare('SELECT carnumber, finishtime'
                         .' FROM RaceChart JOIN RegistrationInfo'
                         .' USING (racerid)'
                         .' WHERE finishtime IS NOT NULL AND finishtime > 0'
                         .' ORDER BY finishtime'
                         .' LIMIT 15');
    $stmt->execute(array());
    json_out('best-times', $stmt->fetchAll(PDO::FETCH_ASSOC));
    break;
  case 'classes':
    require_once('inc/json-classes.inc');
    json_out('classes', json_all_classes());
    break;
  case 'current-heat':
    require_once('inc/json-current-heat.inc');
    json_out('current-heat', json_current_heat($now_running));
    break;
  case 'partitions':
    require_once('inc/json-partitions.inc');
    json_out('partitions', json_partitions());
    break;
  case 'heat-results':
    require_once('inc/json-heat-results.inc');
    if ($now_running['roundid'] == TIMER_TEST_ROUNDID) {
      $tt = timer_test_recorded_results();
      json_out('heat-results', $tt['results']);
    } else {
      // TODO the roundid and heat are passed in to reflect what the requesting
      // page thinks the current round is; both poll.coordinator and
      // poll.now-racing both do this.  This is avoid the server having to work
      // out what the "previous" heat was, and also to overcome the possibility
      // that the heat changed twice between poll requests.
      //
      // Instead of relying on $_GET, pass in a struct with per-key arguments:
      //  { 'heat-results': { roundid: , heat: }
      json_out('heat-results',
               json_heat_results(isset($_GET['roundid']) ? $_GET['roundid'] : '0',
                                 isset($_GET['heat']) ? $_GET['heat'] : '1'));
    }
    break;
  case 'last-heat':
    require_once('inc/json-last-heat.inc');
    json_out('last-heat', json_last_heat());
    break;
  case 'precision':
    json_out('precision', get_finishtime_formatting_string() == "%6.4f" ? 4 : 3);
    break;
  case 'racers':  
    require_once('inc/json-current-racers.inc');
    if ($now_running['roundid'] == TIMER_TEST_ROUNDID) {
      $racers = array();
      $nlanes = get_lane_count();
      for ($lane = 1; $lane <= $nlanes; ++$lane) {
        if (($tt['mask'] & (1 << ($lane - 1))) == 0) {
          $racers[] = array('lane' => $lane);
        }
      }
      json_out('racers', $racers);
    } else {
      require_once('inc/photos-on-now-racing.inc');
      $render = false;
      if (isset($_GET['row-height']) && $_GET['row-height'] > 0) {
        if (isset($_GET['repo'])) {
          $photos = $_GET['repo'];
          if (!photo_repository($photos)) {
            $photos = 0;
          }
        } else {
          $photos = read_photos_on_now_racing();
        }
        if ($photos) {
          $render_name = (2 * $_GET['row-height']).'x'.$_GET['row-height'];
          $render = photo_repository($photos)->lookup_or_any($render_name);
        }
      }

      json_out('racers', json_current_racers($now_running, read_name_style(), $render));
    }
    break;
  case 'replay-state':
    require_once('inc/json-replay.inc');
    json_out('replay-state', json_replay());
    break;
  case 'rounds':
    require_once('inc/json-rounds.inc');
    json_out('rounds', json_all_rounds($now_running));
    break;
  case 'timer-state':
    require_once('inc/json-timer-state.inc');
    json_out('timer-state', json_timer_state());
    break;
  case 'timer-trouble':
    if (warn_no_timer()) {
      require_once('inc/timer-state.inc');
      $timer_state = new TimerState();
      if ($timer_state->troubled()) {
        json_out('timer-trouble', true);
      }
    }
    break;
  }
}

$now_running = get_running_round();
foreach (explode(',', $_GET['values']) as $key) {
  json_common_1($key, $now_running);
}

?>