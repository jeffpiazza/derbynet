<?php
// Generate XML elements for the racers in the current heat
// Expects $now_running array with roundid and heat elements

require_once('inc/data.inc');
require_once('inc/photo-config.inc');
require_once('inc/name-mangler.inc');
require_once('inc/schema_version.inc');


// Pass a photo_render value to include a photo URL for each racer.
// $photo_render if a PhotoRender or false
function emit_current_racers(&$now_running, $name_style, $photo_render) {
  global $db;

  $use_subgroups = use_subgroups();
  $time_format = get_finishtime_formatting_string();

  $stmt = $db->prepare('SELECT lane, carnumber, lastname, firstname, carname, rank,'
                       .' finishtime, finishplace, imagefile, '
                       .(schema_version() >= 2 ? 'carphoto' : "'' AS carphoto")
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
  foreach ($stmt as $row) {
    echo '  <racer lane="'.$row['lane'].'"'
    .' name="'.htmlspecialchars(mangled_name($row, $name_style), ENT_QUOTES, 'UTF-8').'"'
    .($use_subgroups ? ' subgroup="'.htmlspecialchars($row['rank'], ENT_QUOTES, 'UTF-8').'"' : '')
    .' carname="'.htmlspecialchars($row['carname'], ENT_QUOTES, 'UTF-8').'"'
    .' carnumber="'.$row['carnumber'].'"'
    .' photo="'.($photo_render ? $photo_render->url_for_row($row) : '').'"'
    .' finishtime="'.($row['finishtime'] ? sprintf($time_format, $row['finishtime']) : '').'"'
    .' finishplace="'.($row['finishplace'] ? $row['finishplace'] : '').'"'
    .'/>'."\n";
  }
}
?>