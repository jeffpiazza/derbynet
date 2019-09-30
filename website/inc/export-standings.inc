<?php

require_once('inc/data.inc');
require_once('inc/authorize.inc');
require_once('inc/name-mangler.inc');

require_once('inc/standings.inc');


function export_standings(callable $out, $requested_roundid, $requested_rankid) {
  $showing_all = ($requested_roundid === false);

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
  if (!$showing_all) {
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

  $standings = final_standings();
  $standings_by_racer = collect_standings($standings);

  try {
    foreach ($standings as &$row) {
      $racerid = $row['racerid'];
      $roundid = $row['roundid'];
      $classid = $row['classid'];
      $rankid = $row['rankid'];
      $this_racer_standings = $standings_by_racer[$racerid];
    
      $show_this_row = true;
      if ($requested_roundid === false) {
        $show_this_row = $show_this_row && $row['for_supergroup'];
      } else {
        $show_this_row = $show_this_row && ($roundid == $requested_roundid);
      }
      if ($requested_rankid !== false) {
        $show_this_row = $show_this_row && ($rankid == $requested_rankid);
      }

      if ($show_this_row) {
        $values = array();
        if ($showing_all) { // supergroup standings
          $values[] = isset($this_racer_standings['supergroup']) ? $this_racer_standings['supergroup'] : '';
        } else {
          $values[] = $this_racer_standings[$roundid];
        }
        $values[] = $row['carnumber'];
        $values[] = mangled_name($row, $name_style);
        if ($show_car_name) {
          $values[] = $row['carname'];
        }
        if ($use_groups) {
          $values[] = $row['class'];
          $values[] = ($row['for_group'] && isset($this_racer_standings['c'.$classid]))
          ? $this_racer_standings['c'.$classid]
          : '';
        }
        if ($use_subgroups) {
          $values[] = $row['rank'];
          $values[] = ($row['for_group'] && isset($this_racer_standings['r'.$rankid]))
          ? $this_racer_standings['r'.$rankid]
          : '';
        }

        // Place in pack
        if (!$showing_all) {
          $values[] = ($row['for_supergroup'] && isset($this_racer_standings['supergroup']))
          ? $this_racer_standings['supergroup']
          : '';
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
  } catch (Exception $e) {
    echo '<error msg="'.htmlspecialchars($e->getMessage(), ENT_QUOTES, 'UTF-8').'"/>'."\n";
  }
}

?>