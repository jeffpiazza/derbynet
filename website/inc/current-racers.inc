<?php
// Generate XML elements for the racers in the current heat
// Expects $now_running array with roundid and heat elements

require_once('inc/data.inc');
require_once('inc/photo-config.inc');

// Pass a photo_render value to include a photo URL for each racer
function emit_current_racers(&$now_running, $photo_render = false) {
  global $db;

  $use_subgroups = read_raceinfo_boolean('use-subgroups');

  $stmt = $db->prepare('SELECT lane, carnumber, lastname, firstname, carname, rank, imagefile, finishtime'
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
    // TODO: Offer different formats for name
    .' name="'.htmlspecialchars($row['firstname'].' '.$row['lastname'], ENT_QUOTES, 'UTF-8').'"'
    .($use_subgroups ? ' subgroup="'.htmlspecialchars($row['rank'], ENT_QUOTES, 'UTF-8').'"' : '')
    .' carname="'.htmlspecialchars($row['carname'], ENT_QUOTES, 'UTF-8').'"'
    .' carnumber="'.$row['carnumber'].'"'
    .' photo="'.($photo_render ? headshots()->url_for_racer($row, $photo_render) : '').'"'
    .' finishtime="'.($row['finishtime'] ? sprintf('%5.3f', $row['finishtime']) : '').'"'
    .'/>'."\n";
  }
}
?>