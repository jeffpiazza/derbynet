<?php @session_start();
require_once('inc/data.inc');
require_once('inc/authorize.inc');
require_once('inc/name-mangler.inc');

header('Content-Type: text/csv; charset=utf-8');
header('Content-Disposition: attachment; filename="derbynet-'.date('Y-m-d').'.csv"');

$name_style = isset($_GET['last_initial']) ? FIRST_NAME_LAST_INITIAL : FULL_NAME;

echo "Class,Round,Heat,Lane,";
if ($name_style == FULL_NAME) {
  echo "FirstName,LastName,";
} else {  // FIRST_NAME_LAST_INITIAL
  echo "Name,";
}
echo "CarNumber,CarName,FinishTime,FinishPlace,Completed\r\n";

$stmt = $db->query('SELECT class, round, heat, lane,'
                   .' firstname, lastname, carnumber, carname, finishtime, finishplace, completed'
                   .' FROM '.inner_join('RaceChart', 'RegistrationInfo',
                                        'RaceChart.racerid = RegistrationInfo.racerid',
                                        'Rounds', 'RaceChart.roundid = Rounds.roundid',
                                        'Classes',
                                        'Rounds.classid = Classes.classid')
                   .' ORDER BY completed, heat, lane');
if ($stmt === FALSE) {
	$info = $db->errorInfo();
    echo '<error msg="'.htmlspecialchars($info[2], ENT_QUOTES, 'UTF-8').'" query="'.$sql.'"/>'."\n";
}

$output = fopen("php://output", "w");
try {
  foreach ($stmt as $row) {
    $values = array($row['class'], $row['round'], $row['heat'], $row['lane']);
    if ($name_style == FIRST_NAME_LAST_INITIAL) {
      array_push($values, mangled_name($row, $name_style));
    } else {
      array_push($values, $row['firstname'], $row['lastname']);
    }
    array_push($values, $row['carnumber'], $row['carname'],
               $row['finishtime'], $row['finishplace'], $row['completed']);
    fputcsv($output, $values);
  }
} catch (Exception $e) {
  echo '<error msg="'.htmlspecialchars($e->getMessage(), ENT_QUOTES, 'UTF-8').'"/>'."\n";
}

// finally clause syntax is recognized only in PHP 5.5 and later
fclose($output);
?>
