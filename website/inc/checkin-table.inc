<?php

require_once('inc/classes.inc');
require_once('inc/photo-config.inc');
require_once('inc/schema_version.inc');
require_once('inc/xbs.inc');

function checkin_table_SELECT_FROM_sql() {
  $xbs_award_name = xbs_award();
  $schema_version = schema_version();
  return
    'SELECT racerid, carnumber, lastname, firstname, carname, imagefile,'
    .($schema_version < 2 ? " '' AS carphoto," : " carphoto,")
    .($schema_version < PARTITION_SCHEMA
      ? " 1 AS partitionid, 1 AS partition_sortorder, '".DEFAULT_PARTITION_NAME."' AS partition_name,"
      ." '' AS note,"
      : " Partitions.partitionid AS partitionid,"
        ." Partitions.sortorder AS partition_sortorder,"
        ." Partitions.name AS partition_name, note,")
    .' RegistrationInfo.classid, class, RegistrationInfo.rankid, rank, passedinspection, exclude,'
    .' EXISTS(SELECT 1 FROM RaceChart WHERE RaceChart.racerid = RegistrationInfo.racerid) AS scheduled,'
    .' EXISTS(SELECT 1 FROM RaceChart WHERE RaceChart.classid = RegistrationInfo.classid) AS denscheduled,'
    .' EXISTS(SELECT 1 FROM Awards WHERE Awards.awardname = \''.addslashes($xbs_award_name).'\' AND'
    .'                                   Awards.racerid = RegistrationInfo.racerid) AS xbs'
    .' FROM '.($schema_version < PARTITION_SCHEMA 
               ? inner_join('RegistrationInfo', 'Classes',
                            'RegistrationInfo.classid = Classes.classid',
                            'Ranks',
                            'RegistrationInfo.rankid = Ranks.rankid')
               : inner_join('RegistrationInfo', 'Classes',
                            'RegistrationInfo.classid = Classes.classid',
                            'Ranks',
                            'RegistrationInfo.rankid = Ranks.rankid',
                            'Partitions',
                            'RegistrationInfo.partitionid = Partitions.partitionid'));
 }

function json_table_row_for_id($racerid) {
  $xbs_award_name = xbs_award();
  $schema_version = schema_version();
  $row = read_single_row(
    checkin_table_SELECT_FROM_sql().' WHERE RegistrationInfo.racerid = :racerid',
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
    'partitionid' => $row['partitionid'],
    'partition' => $row['partition_name'],
    'partition_sortorder' => $row['partition_sortorder'],
    'note' => /* is_null($row['note']) ? '' :*/ $row['note'],
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