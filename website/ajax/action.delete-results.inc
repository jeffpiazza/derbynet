<?php
// $_POST['roundid'] = number, or 'all'
// if $_POST['heat'], then just delete results for that heat only

start_response();

if (have_permission(CONTROL_RACE_PERMISSION)) {
  $params = array();
  $sql = 'UPDATE RaceChart SET finishtime = NULL, finishplace = NULL, completed = NULL';
  $roundid = isset($_POST['roundid']) ? $_POST['roundid'] : 'current';

  if ($roundid == 'all') {
    // Don't need a more specific query
  } else {
    $current = get_running_round();
    if ($roundid == 'current') {
      $roundid = $current['roundid'];
    }
    $sql .= ' WHERE roundid = :roundid';
    $params[':roundid'] = $roundid;
    if (isset($_POST['heat'])) {
      $heat = $_POST['heat'];
      if ($heat == 'current') {
        $heat = $current['heat'];
      }
      $sql .= ' AND heat = :heat';
      $params[':heat'] = $heat;
    }
  }

  $stmt = $db->prepare($sql);
  $stmt->execute($params);

  echo "\n<success/>\n";
  echo "<sql>".htmlspecialchars($sql, ENT_QUOTES, 'UTF-8')."</sql>\n";

  require('ajax/query.poll.coordinator.inc');

} else {
  not_authorized_body();
}

end_response();