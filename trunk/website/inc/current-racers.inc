<?php
// Generate XML elements for the racers in the current heat
// Expects $now_running array with roundid and heat elements

require_once('inc/data.inc');
require_once('inc/photo-config.inc');

$show_racer_photos = read_raceinfo_boolean('show-racer-photos');
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
        .' photo="'.($show_racer_photos ? headshots()->url_for_racer($row) : '').'"'
        .' finishtime="'.($row['finishtime'] ? sprintf('%5.3f', $row['finishtime']) : '').'"'
        .'/>'."\n";
}
?>