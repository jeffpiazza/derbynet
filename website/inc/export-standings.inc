<?php

require_once('inc/data.inc');
require_once('inc/authorize.inc');
require_once('inc/name-mangler.inc');

require_once('inc/standings.inc');

function export_standings(callable $out, callable $choose, $key,
                          &$result_summary, &$parallel) {
  $showing_supergroup = $key == 'supergroup';
  $name_style = read_raceinfo('name-style', FULL_NAME);
  $use_groups = show_group_standings();
  $use_subgroups = read_raceinfo_boolean('use-subgroups');
  $show_car_name = show_car_name_in_standings();
  $use_points = read_raceinfo_boolean('use-points');
  $time_format = get_finishtime_formatting_string();

  $out(array($use_points
             ? "Scoring by points"
             : (read_raceinfo_boolean('drop-slowest')
                ? "Dropping each racer's slowest time"
                : "Averaging all heat times")));

  $headers = array("Place", "Car Number", "Name");
  if (show_car_name_in_standings()) {
    $headers[] = "Car Name";
  }
  if ($use_groups) {
    $headers[] = group_label();
    $headers[] = "In ".group_label();
  }
  if ($use_subgroups) {
    $headers[] = subgroup_label();
    $headers[] = "In ".subgroup_label();
  }

  if (!$showing_supergroup) {
    // If we're showing overall standings, then the standing in the pack is given
    // in the first column, and this would be redundant.
    $headers[] = "In ".supergroup_label();
  }
  $headers[] = "Heats";
  if (read_raceinfo_boolean('use-points')) {
    $headers[] = "Total Points (1st = ".get_lane_count().")";
  } else {
    $headers[] = "Average";
  }
  $headers[] = "Best";
  $headers[] = "Worst";
  $out($headers);

  $i = 0;
  foreach ($result_summary as $row) {
    $p = $parallel[$i];
    ++$i;

    $racerid = $row['racerid'];
    $roundid = $row['roundid'];
    $classid = $row['classid'];
    $rankid = $row['rankid'];

    if ($choose($row, $p)) {
      $values = array();
      $values[] = isset($p[$key]) ? $p[$key] : '';

      $values[] = $row['carnumber'];
      $values[] = mangled_name($row, $name_style);
      if ($show_car_name) {
        $values[] = $row['carname'];
      }
      if ($use_groups) {
        $values[] = $row['class'];
        $values[] = isset($p['c'.$classid]) ? $p['c'.$classid] : '';
      }
      if ($use_subgroups) {
        $values[] = $row['rank'];
        $values[] = isset($p['r'.$rankid]) ? $p['r'.$rankid] : '';
      }

      if (!$showing_supergroup) {
        $values[] = isset($p['supergroup']) ? $p['supergroup'] : '';
      }

      $values[] = $row['base'];  // Number of heats

      if ($use_points) {
        $values[] = $row['avg'];
        $values[] = ordinal($row['best']);
        $values[] = ordinal($row['worst']);
      } else {
        $values[] = sprintf($time_format, $row['avg']);
        $values[] = sprintf($time_format, $row['best']);
        $values[] = sprintf($time_format, $row['worst']);
      }

      $out($values);
    }
  }
}

?>