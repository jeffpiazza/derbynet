<?php

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
   RegistrationInfo(rankid) -> Ranks

   Roster(roundid) -> Rounds
   Roster(classid) -> Classes
   Roster(racerid) -> RegistrationInfo

   Rounds(classid) -> Classes
 */

$schema = array('Classes' => array('classid', 'class'),
                // TODO: sortorder for Classes, Ranks
                'Ranks' => array('rankid', 'rank', 'classid'),
                // TODO: carphoto for RegistrationInfo
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
?>