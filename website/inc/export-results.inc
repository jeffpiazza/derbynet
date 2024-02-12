<?php
require_once('inc/name-mangler.inc');

// $output is a file handle for writing
function export_results(callable $out) {
  global $db;

  $name_style = read_name_style();

  $headings = array();
  $headings[] = "Class";
  $headings[] = "Round";
  $headings[] = "Heat";
  $headings[] = "Lane";
  if ($name_style == FULL_NAME) {
    $headings[] = "FirstName";
    $headings[] = "LastName";
  } else {
    $headings[] = "Name";
  }
  $headings[] = "CarNumber";
  $headings[] = "CarName";
  $headings[] = "FinishTime";
  $headings[] = "Scale MPH";
  $headings[] = "FinishPlace";
  $headings[] = "Completed";

  $out($headings);

  $track_length = read_raceinfo('track-length', 40);  // in feet
  // $track_length / $finishtime gives feet per second.
  //  * 3600 gives feet per hour
  //  / 5280 gives miles per hour
  //  * 25 gives scale miles per mile (not user-adjustable; makes a 7" derby car scale to 14.5 feet)
  $fps_to_scale_mph = 3600 * 25 / 5280;

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

  try {
    foreach ($stmt as $row) {
      $values = array($row['class'], (int) $row['round'], (int) $row['heat'], (int) $row['lane']);
      if ($name_style == FIRST_NAME_LAST_INITIAL) {
        array_push($values, mangled_name($row, $name_style));
      } else {
        array_push($values, $row['firstname'], $row['lastname']);
      }
      array_push($values, $row['carnumber'], $row['carname'],
                 (float) $row['finishtime']);
      // Scale MPH
      array_push($values, (!is_numeric($row['finishtime']) || $row['finishtime'] == 0) ? "--"
                 : (float) sprintf("%4.1f", $track_length / $row['finishtime'] * $fps_to_scale_mph));
      array_push($values, $row['finishplace'], $row['completed']);
                 
      $out($values);
    }
  } catch (Exception $e) {
    echo '<error msg="'.htmlspecialchars($e->getMessage(), ENT_QUOTES, 'UTF-8').'"/>'."\n";
  }
}




function export_results_csv($output, $name_style = FULL_NAME) {
  global $db;

  $headings = array();
  $headings[] = "Class";
  $headings[] = "Round";
  $headings[] = "Heat";
  $headings[] = "Lane";
  if ($name_style == FULL_NAME) {
    $headings[] = "FirstName";
    $headings[] = "LastName";
  } else {
    $headings[] = "Name";
  }
  $headings[] = "CarNumber";
  $headings[] = "CarName";
  $headings[] = "FinishTime";
  $headings[] = "FinishPlace";
  $headings[] = "Completed";

  fputcsv($output, $headings);

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

  try {
    foreach ($stmt as $row) {
      $values = array($row['class'], $row['round'], $row['heat'], $row['lane']);
      if ($name_style == FIRST_NAME_LAST_INITIAL) {
        array_push($values, mangled_name($row, $name_style));
      } else {
        array_push($values, $row['firstname'], $row['lastname']);
      }
      array_push($values, $row['carnumber'], $row['carname'],
                 (float) $row['finishtime'], $row['finishplace'], $row['completed']);
      fputcsv($output, $values);
    }
  } catch (Exception $e) {
    echo '<error msg="'.htmlspecialchars($e->getMessage(), ENT_QUOTES, 'UTF-8').'"/>'."\n";
  }

  // Syntax for finally clause is recognized only in PHP 5.5 and later
  fclose($output);
}
?>
