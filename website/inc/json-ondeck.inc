<?php

require_once('inc/autoadvance.inc');
require_once('inc/name-mangler.inc');
require_once('inc/ordinals.inc');
require_once('inc/photo-config.inc');
require_once('inc/schema_version.inc');

// TODO Not racing yet (no current heat)?
//
function json_ondeck(&$now_running) {
  global $db;

  $use_master_sched = $now_running['use_master_sched'];
  $use_points = $now_running['use_points'];
  $name_style = read_name_style();
  $time_format = get_finishtime_formatting_string();

  $ondeck = array();

  $params = array();
  if ($use_master_sched) {
    $where = 'round = :round';
    $params[':round'] = $now_running['round'];
    $ondeck['title'] = "Round $now_running[round]";
  } else {
    $where = 'roundid = :roundid';
    $params[':roundid'] = $now_running['roundid'];
    $ondeck['title'] = "$now_running[class], Round $now_running[round]";
  }

  $next = get_next_heat($now_running, 'next');
  if ($next !== false) {
    $ondeck['next'] = array('roundid' => $next['roundid'],
                            'heat' => $next['heat']);
  }

  $stmt = $db->prepare('SELECT resultid, roundid, class, heat, lane,'
                       .' racerid, firstname, lastname, carnumber,'
                       .(schema_version() < 2 ? ' \'\' as' : '').' carphoto, '
                       .($use_master_sched ? 'masterheat' : 'heat').' as seq,'
                       .' finishtime, finishplace, completed'
                       .' FROM RaceChart'
                       .' INNER JOIN Rounds USING (roundid)'
                       .' INNER JOIN Classes USING (classid)'
                       .' INNER JOIN RegistrationInfo USING (racerid)'
                       .' WHERE '.$where
                       .' ORDER BY seq, lane');

  $stmt->execute($params);
  $chart = array();

  $render = car_photo_repository()->lookup(RENDER_CROPPED);
  
  foreach ($stmt as $row) {
    $chart[] = array('resultid' => $row['resultid'],
                     'roundid' => $row['roundid'],
                     'class' => $row['class'],
                     'heat' => $row['heat'],
                     'lane' => $row['lane'],
                     'racerid' => $row['racerid'],
                     'name' => mangled_name($row, $name_style),
                     'carnumber' => $row['carnumber'],
                     'result' => (empty($row['completed'])
                                  ? ''
                                  : ($use_points
                                     ? 'z'.ordinal($row['finishplace'])
                                     : 'z'.sprintf($time_format, $row['finishtime']))),
                     'carphoto' => $render->url_elements_for_row($row)
      );
  }
  $ondeck['chart'] = $chart;
  return $ondeck;
}


?>