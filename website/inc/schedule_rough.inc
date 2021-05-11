<?php

$all_scheduler_data = array(1 => array(), 2 => array(), 3 => array(),
                            4 => array(), 5 => array(), 6 => array(),
                            7 => array(), 8 => array());

function make_scheduler_data($nlanes, $ncars, $generators, ...$permutation_sets) {
  global $all_scheduler_data;
  $all_scheduler_data[$nlanes][$ncars] =
      array('generators' => $generators,
            'permutation_sets' => $permutation_sets);
}

require_once('inc/generators/2-lanes.inc');
require_once('inc/generators/3-lanes.inc');
require_once('inc/generators/4-lanes.inc');
require_once('inc/generators/5-lanes.inc');
require_once('inc/generators/6-lanes.inc');
require_once('inc/generators/8-lanes.inc');

// Returns an array of generators, appropriate for $nlanes and $ncars.
function get_scheduler_data($nlanes, $ncars, $max_runs_per_car) {  
  global $all_scheduler_data;
  // If there's a non-zero max-runs-per-car value, then we adopt a simple rotation schedule,
  // which means a generator value of 1 for each lane.
  if ($max_runs_per_car != 0) {
    $generators = array();
    for ($i = 0; $i < $nlanes - 1; ++$i) {
      $generators[] = 1;
    }

    return array('generators' => array($generators),
                 'permutation_sets' => array());
  }

  if ($nlanes == 1) {
    return array('generators' => array(array(), array(), array(),
                                       array(), array(), array()),
                 'permutation_sets' => array());
  }

  return $all_scheduler_data[$nlanes][$ncars];
}

function apply_generator($gen, $nlanes, $ncars, &$heats) {
  for ($h = 0; $h < $ncars; ++$h) {
    $heat = array();
    $heat[] = $h;
    for ($lane = 1; $lane < $nlanes; ++$lane) {
      $heat[] = ($heat[$lane - 1] + $gen[$lane - 1]) % $ncars;
    }
    $heats[] = $heat;
  }
}

function apply_permutation(&$permutation, $i_begin, $i_end, &$heats) {
  for ($i = $i_begin; $i < $i_end; ++$i) {
    $heat = array();
    foreach ($heats[$i] as $car) {
      $heat[] = $permutation[$car];
    }
    $heats[] = $heat;
  }
}

// The rough schedule is an array of heats.  Each heat is an array of length
// $nlanes, each entry being a value c, 0 <= c < $ncars.  (c is not a racerid.)
function make_rough_schedule($nlanes, $ncars, $n_times_per_lane, $max_runs_per_car, $trace) {
  $data = get_scheduler_data($nlanes, $ncars, $max_runs_per_car);
  $generators = $data['generators'];

  $heats = array();

  for ($gi = 0; $gi < count($generators) && $n_times_per_lane > 0; ++$gi) {
    apply_generator($generators[$gi], $nlanes, $ncars, $heats);
    --$n_times_per_lane;
    if ($trace) {
      $trace[] = array('generator' => $gi,
                       'heats' => array_slice($heats, $gi * $ncars, $ncars));
    }
  }

  if ($n_times_per_lane > 0) {
    $psets = $data['permutation_sets'];

    $k = 0;
    $making_progress = true;
    while ($n_times_per_lane > 0 && $making_progress) {
      $making_progress = false;
      for ($gi = 0; $n_times_per_lane > 0 && $gi < count($generators) && $gi < count($psets); ++$gi) {
        if ($k < count($psets[$gi])) {
          apply_permutation($psets[$gi][$k], $gi * $ncars, ($gi + 1) * $ncars, $heats);
          if ($trace) {
            $trace[] = array('permutation' => $k,
                             'generator' => $gi,
                             'psets' => $psets[$gi][$k],
                             'heats' => array_slice($heats, count($heats) - $ncars));
          }
          --$n_times_per_lane;
          $making_progress = true;
        }
      }
      ++$k;
    }
  }

  if ($trace) {
    $trace[] = array('rough-schedule' => $heats);
  }

  if ($n_times_per_lane > 0) {
    json_failure('cant-fulfill', "Can't fulfill");
    return array();
  }

  return $heats;
}

?>