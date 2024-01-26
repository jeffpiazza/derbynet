<?php

json_out('timestamp', array('ymd' => date('Y-m-d H:i:s'),
                            'time' => time()));

$race_info = array();
foreach ($db->query('SELECT * FROM RaceInfo') as $row) {
  $race_info[$row['itemkey']] = $row['itemvalue'];
}
json_out('race_info', $race_info);

json_out('racers',
         array('total' => read_single_value('SELECT COUNT(*) FROM RegistrationInfo'),
               'passed' => read_single_value('SELECT COUNT(*) FROM RegistrationInfo'
                                             .' WHERE passedinspection'),
               'racerphoto' => read_single_value('SELECT COUNT(*) FROM RegistrationInfo'
                                                 .' WHERE imagefile IS NOT NULL'
                                                 .'  AND  imagefile <> \'\''),
               'carphoto' => read_single_value('SELECT COUNT(*) FROM RegistrationInfo'
                                               .' WHERE carphoto IS NOT NULL'
                                               .'  AND  carphoto <> \'\'')));

json_out('heats',
         array('scheduled' =>
                 read_single_value('SELECT COUNT(*) FROM (SELECT DISTINCT roundid, heat FROM RaceChart)'),
               'raced'=>
                 read_single_value('SELECT COUNT(*) FROM (SELECT DISTINCT roundid, heat FROM RaceChart'
                                   .' WHERE completed IS NOT NULL'
                                   .'  AND (finishtime IS NOT NULL OR'
                                   .'       finishplace IS NOT NULL))')));

json_out('latest_heat_run',
         read_single_value('SELECT MAX(completed) FROM RaceChart'
                           .' WHERE completed IS NOT NULL AND'
                           .'   (finishtime IS NOT NULL OR finishplace IS NOT NULL)'));

json_out('voting',
         array('n_ballot_awards' => read_single_value('SELECT COUNT(*) FROM BallotAwards'),
               'n_ballot_unawarded' =>
                 read_single_value('SELECT COUNT(*) FROM Awards'
                                   .' WHERE awardid IN (SELECT awardid FROM BallotAwards)'
                                   .' AND (racerid IS NULL OR racerid = 0)'),
               'nvotes' => read_single_value('SELECT COUNT(*) FROM Votes')));

json_success();

?>
