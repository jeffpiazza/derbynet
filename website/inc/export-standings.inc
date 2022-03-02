<?php

require_once('inc/data.inc');
require_once('inc/authorize.inc');
require_once('inc/name-mangler.inc');
require_once('inc/partitions.inc');

require_once('inc/standings.inc');

if (read_raceinfo('drop-slowest') && read_raceinfo('scoring', -1) == -1) {
  write_raceinfo('scoring', 1);
}

function export_standings(callable $out, $key, $presentation,
                          &$result_summary) {
  $name_style = read_name_style();
  $use_groups = use_groups();
  $use_subgroups = use_subgroups();
  $show_car_name = show_car_name_in_standings();
  $use_points = read_raceinfo_boolean('use-points');
  $time_format = get_finishtime_formatting_string();
  $scoring = read_raceinfo('scoring', 0);

  $show_supergroup_column = $presentation == 'subgroup';
  $show_inround_column = false;  // TODO

  $out(array($use_points ? "Scoring by points"
             : ($scoring == 0 ? "Averaging all heat times"
             : ($scoring == 1 ? "Dropping each racer's slowest time"
             : ($scoring == 2 ? "Using each racer's fastest time only"
             : "Unknown scoring method")))));

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

  if ($show_supergroup_column) {
    // If we're showing overall standings, then the standing in the pack is given
    // in the first column, and this would be redundant.
    $headers[] = "In ".supergroup_label();
  }
  if ($show_inround_column) {
    $headers[] = "In Round";
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
    $p = $row['outcome'];
    ++$i;

    $racerid = $row['racerid'];
    $roundid = $row['roundid'];
    $classid = $row['classid'];
    $rankid = $row['rankid'];

    if (isset($p[$key])) {
      $values = array();
      $values[] = isset($p[$key]) ? $p[$key] : '';

      $values[] = $row['carnumber'];
      $values[] = mangled_name($row, $name_style);
      if ($show_car_name) {
        $values[] = $row['carname'];
      }
      if ($use_groups) {
        $values[] = $row['class'];
        $values[] = ($presentation == 'ff' || $presentation == 'class' || $presentation == 'subgroup')
        ? $p[stkey_round($roundid)] : '';
      }
      if ($use_subgroups) {
        $values[] = $row['rank'];
        $values[] = $p[stkey_round_subgroup($roundid, $rankid)];
      }

      if ($show_supergroup_column) {
        $values[] = isset($p[stkey_full_field()]) ? $p[stkey_full_field()] : '';
      }
      if ($show_inround_column) {
        $values[] = $p[stkey_round($roundid)];
      }

      $values[] = $row['base'];  // Number of heats

      if ($use_points) {
        $values[] = $row['avg'];
        $values[] = ordinal($row['best']);
        $values[] = ordinal($row['worst']);
      } else {
        $values[] = (float) sprintf($time_format, $row['avg']);
        $values[] = (float) sprintf($time_format, $row['best']);
        $values[] = (float) sprintf($time_format, $row['worst']);
      }

      $out($values);
    }
  }
}

?>