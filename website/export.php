<?php @session_start();
require_once('inc/data.inc');
require_once('inc/authorize.inc');

header('Content-Type: text/csv; charset=utf-8');

echo "Class,Round,Heat,Lane,FirstName,LastName,CarNumber,FinishTime,FinishPlace,Completed\r\n";

$stmt = $db->query('SELECT class, round, heat, lane,'
                   .' firstname, lastname, carnumber, finishtime, finishplace, completed'
                   .' FROM '.inner_join('RaceChart', 'RegistrationInfo',
                                        'RaceChart.racerid = RegistrationInfo.racerid',
                                        'Rounds', 'RaceChart.roundid = Rounds.roundid',
                                        'Classes',
                                        'Rounds.classid = Classes.classid')
                   .' ORDER BY completed, heat, lane');
if ($stmt === FALSE) {
	$info = $db->errorInfo();
    echo '<error msg="'.$info[2].'" query="'.$sql.'"/>'."\n";
  }

$output = fopen("php://output", "w");
try {
  foreach ($stmt as $row) {
    fputcsv($output, array($row['class'], $row['round'], $row['heat'], $row['lane'],
                           $row['firstname'], $row['lastname'], $row['carnumber'],
                           $row['finishtime'], $row['finishplace'], $row['completed']));
  }
} finally {
  fclose($output);
}

