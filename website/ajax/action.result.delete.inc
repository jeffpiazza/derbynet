<?php
// $_POST['roundid'] = number, or 'all'
// if $_POST['heat'], then just delete results for that heat only
require_once('inc/events.inc');
require_once('inc/event-lane-details.inc');

if (have_permission(CONTROL_RACE_PERMISSION)) {
  $params = array();
  $sql = 'UPDATE RaceChart SET finishtime = NULL, finishplace = NULL, completed = NULL';
  $roundid = isset($_POST['roundid']) ? $_POST['roundid'] : 'current';

  if ($roundid == 'all') {
    // Don't need a more specific query
    $where = '';
  } else {
    $current = get_running_round();
    if ($roundid == 'current') {
      $roundid = $current['roundid'];
    }
    $where = ' WHERE roundid = :roundid';
    $params[':roundid'] = $roundid;
    if (isset($_POST['heat'])) {
      $heat = $_POST['heat'];
      if ($heat == 'current') {
        $heat = $current['heat'];
      }
      $where .= ' AND heat = :heat';
      $params[':heat'] = $heat;
    }
  }

  if (is_int($roundid) && is_int($heat)) {
    // TODO Cover other cases, too.
    record_event_for_discarded_heat($roundid, $heat);
  }

  $sql .= $where;
  $stmt = $db->prepare($sql);
  $stmt->execute($params);

  json_success();
  json_out('sql', $sql);

  require('ajax/query.poll.coordinator.inc');

} else {
  json_not_authorized();
}
