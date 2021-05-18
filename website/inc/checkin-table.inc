<?php

require_once('inc/classes.inc');
require_once('inc/photo-config.inc');
require_once('inc/schema_version.inc');
require_once('inc/xbs.inc');

function json_table_row_for_id($racerid) {
  $xbs_award_name = xbs_award();
  $row = read_single_row(
    'SELECT racerid, carnumber, lastname, firstname, carname, imagefile,'
      .(schema_version() < 2 ? "" : " carphoto,")
    .' RegistrationInfo.classid, class, RegistrationInfo.rankid, rank, passedinspection, exclude,'
    .' EXISTS(SELECT 1 FROM RaceChart WHERE RaceChart.racerid = RegistrationInfo.racerid) AS scheduled,'
    .' EXISTS(SELECT 1 FROM RaceChart WHERE RaceChart.classid = RegistrationInfo.classid) AS denscheduled,'
    .' EXISTS(SELECT 1 FROM Awards WHERE Awards.awardname = \''.addslashes($xbs_award_name).'\' AND'
    .'                                   Awards.racerid = RegistrationInfo.racerid) AS xbs'
    .' FROM '.inner_join('RegistrationInfo', 'Classes',
                         'RegistrationInfo.classid = Classes.classid',
                         'Ranks',
                         'RegistrationInfo.rankid = Ranks.rankid')
    .' WHERE RegistrationInfo.racerid = :racerid',
    array(':racerid' => $racerid),
    PDO::FETCH_ASSOC);

  list($classes, $classseq, $ranks, $rankseq) = classes_and_ranks();
  $row['rankseq'] = $ranks[$row['rankid']]['seq'];

  return json_table_row($row, use_groups(), use_subgroups(), !empty($xbs_award_name));
}

function json_table_row(&$row, $n = 0) {
  return array(
    'racerid' => $row['racerid'],
    'rowno' => $n,
    'denscheduled' => $row['denscheduled'] ? true : false,
    'exclude' => $row['exclude'] ? true : false,

    'classid' => $row['classid'],
    'class' => $row['class'],
    'rankid' => $row['rankid'],
    'rank' => $row['rank'],
    'rankseq' => $row['rankseq'],

    'carnumber' => $row['carnumber'],
    'headshot' => $row[headshots()->column_name()]
            ? headshots()->url_for_racer($row, RENDER_CHECKIN)
            : 'img/photo-headshot.png',
    'carphoto' => schema_version() >= 2 && $row[car_photo_repository()->column_name()]
            ? car_photo_repository()->url_for_racer($row, RENDER_CHECKIN)
            : 'img/photo-car.png',

    'firstname' => $row['firstname'],
    'lastname' => $row['lastname'],
    'carname' => $row['carname'],

    'scheduled' => $row['scheduled'] ? true : false,
    'passed' => $row['passedinspection'] ? true : false,

    'xbs' => $row['xbs']);
}

?>