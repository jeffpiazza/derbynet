<?php
require_once('inc/schema_version.inc');

/* Foreign keys:

   Awards(awardtypeid) -> AwardType
   Awards(rankid) -> Ranks
   Awards(classid) -> Classes
   Awards(racerid) -> RegistrationInfo

   RaceChart(classid) -> Classes
   RaceChart(roundid) -> Rounds
   RaceChart(racerid) -> RegistrationInfo

   Ranks(classid) -> Classes

   RegistrationInfo(classid) -> Classes
   RegistrationInfo(divisionid) -> Divisions
   RegistrationInfo(rankid) -> Ranks
   RegistrationInfo(subdivisionid) -> Subdivisions

   Roster(roundid) -> Rounds
   Roster(classid) -> Classes
   Roster(racerid) -> RegistrationInfo

   Rounds(classid) -> Classes

   SceneKiosk(sceneid) -> Scenes

   Subdivisions(superdiv) -> Divisions

   Playlist(roundid) -> Rounds
   Playlist(sceneid_at_finish) -> Scenes

   BallotAwards(awardid) -> Awards

   Votes(awardid) -> Awards
   Votes(racerid) -> RegistrationInfo
   Votes(voterid) -> Voters
 */

$schema = array('Classes' => array('classid', 'class'),
                'Ranks' => array('rankid', 'rank', 'classid'),
                'RegistrationInfo' => array('racerid', 'carnumber', 'carname',
                                            'lastname', 'firstname', 'classid', 'rankid',
                                            'passedinspection', 'imagefile', 'exclude'),
                'Rounds' => array('roundid', 'round', 'classid', 'charttype', 'phase'),
                'AwardTypes' => array('awardtypeid', 'awardtype'),
                'Awards' => array('awardid', 'awardname', 'awardtypeid', 'classid', 'rankid',
                                  'racerid', 'sort'),
                'Roster' => array('rosterid', 'roundid', 'classid', 'racerid', 'finalist', 'grandfinalist'),
                'RaceChart' => array('resultid', 'classid', 'roundid', 'heat', 'lane', 'racerid',
                                     'chartnumber', 'finishtime', 'finishplace', 'points',
                                     'completed', 'ignoretime', 'masterheat'),
                'RaceInfo' => array('raceinfoid', 'itemkey', 'itemvalue'),
  );

if (schema_version() >= 2) {
  $schema['Classes'][] = 'sortorder';
  $schema['Ranks'][] = 'sortorder';
  $schema['RegistrationInfo'][] = 'carphoto';
}

if (schema_version() >= 3) {
  $schema['Classes'][] = 'constituents';
  $schema['Classes'][] = 'durable';
}

if (schema_version() >= 4) {
  $schema['SceneKiosk'] = array('scenekioskid', 'sceneid', 'kiosk_name', 'page');
  $schema['Scenes'] = array('sceneid', 'name', 'sortorder');
  $schema['Playlist'] = array('queueid', 'seq', 'classid', 'round',
                              'bucket_limit', 'bucketed', 'n_times_per_lane',
                              'sceneid_at_finish', 'continue_racing');
}

if (schema_version() >= 5) {
  $schema['Classes'][] = 'rankids';
}

if (schema_version() >= 6) {
  $schema['BallotAwards'] = array('ballotid', 'awardid', 'depth');
  $schema['Voters'] = array('voterid', 'ballotid', 'when_cast', 'ip_addr', 'identifier');
  $schema['Votes'] = array('voteid', 'awardid', 'racerid', 'voterid');
}

if (schema_version() >= 8) {
  $schema['Divisions'] = array('divisionid', 'name', 'sortorder');
  $schema['Subdivisions'] = array('subdivisionid', 'name', 'superdiv', 'sortorder');
  $schema['RegistrationInfo'][] = 'divisionid';
  $schema['RegistrationInfo'][] = 'subdivisionid';
}

?>