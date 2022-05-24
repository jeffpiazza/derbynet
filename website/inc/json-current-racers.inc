<?php

require_once('inc/data.inc');
require_once('inc/photo-config.inc');
require_once('inc/name-mangler.inc');
require_once('inc/schema_version.inc');

// Expects $now_running array with roundid and heat elements

// Pass a photo_render value to include a photo URL for each racer.
// $photo_render if a PhotoRender or false
function json_current_racers(&$now_running, $name_style, $photo_render, $photo_render2 = false) {
  global $db;
  
  $use_subgroups = use_subgroups();
  $time_format = get_finishtime_formatting_string();

  $stmt = $db->prepare('SELECT lane, RegistrationInfo.racerid,'
                       .' carnumber, lastname, firstname, carname, rank,'
                       .' finishtime, finishplace, imagefile, '
                       .(schema_version() >= 2 ? 'carphoto' : "'' AS carphoto").', '
                       .(schema_version() >= PARTITION_SCHEMA ? ' note' : "'' AS note")
                       .' FROM '
                       .inner_join('RaceChart', 'RegistrationInfo',
                                   'RaceChart.racerid = RegistrationInfo.racerid',
                                   'Ranks',
                                   'RegistrationInfo.rankid = Ranks.rankid')
                       .' WHERE roundid = :roundid'
                       .' AND heat = :heat'
                       .' ORDER BY lane');
  $stmt->execute(array(':roundid' => $now_running['roundid'],
                       ':heat' => $now_running['heat']));
  $racers = array();

  foreach ($stmt as $row) {
    $racer = array(
      'lane' => $row['lane'],
      'racerid' => $row['racerid'],
      'name' => mangled_name($row, $name_style),
      'carname' => $row['carname'],
      'carnumber' => $row['carnumber'],
      'note' => $row['note'],
      'photo' => $photo_render ? $photo_render->url_for_row($row) : '',
      'finishtime' => $row['finishtime'] ? sprintf($time_format, $row['finishtime']) : '',
      'finishplace' => $row['finishplace'] ? $row['finishplace'] : '');
    if ($photo_render2) {
      $racer['photo2'] = $photo_render2->url_for_row($row);
    }
    if ($use_subgroups) {
      $racer['subgroup'] = $row['rank'];
    }
    $racers[] = $racer;
  }

  return $racers;
}
?>