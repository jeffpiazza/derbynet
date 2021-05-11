<?php

require_once('inc/schedule_rough.inc');

// RACE_COUNT_WEIGHT is the weight (penalty) for uneven race counts.  That is,
// when some racers have raced a lot more than others, RACE_COUNT_WEIGHT scales
// the deviation from a preferred condition of having the same race count for
// all racers.
define('RACE_COUNT_WEIGHT', 10);
// CONSECUTIVE_WEIGHT is the weight applied when a car appears in consecutive
// heats.
define('CONSECUTIVE_WEIGHT', 1000);
// CONSECUTIVE_WEIGHT + REPEAT_WEIGHT is the weight if a car appears in the same
// lane in consecutive heats.
define('REPEAT_WEIGHT', 1000);


// $appearances[], indexed by racer (0 .. $ncars - 1), counts the
// number of heats that include that racer.
function rate_race_counts(&$next_heat, $n_prev_heats, &$appearances) {
  $nlanes = count($next_heat);
  $ncars = count($appearances);
  $new_appearances = $appearances;  // Copies $appearances
  foreach ($next_heat as $car) {
    ++$new_appearances[$car];
  }

  $dev = 0;
  // For any car, $target tells how many appearances we'd expect after
  // $next_heat.
  $target = (($n_prev_heats + 1) * $nlanes) / $ncars;
  foreach ($new_appearances as $app) {
    $dev += ($app - $target) * ($app - $target);
  }
  return $dev * RACE_COUNT_WEIGHT;
}

// Assess penalties for having the same car(s) appear in consecutive heats.
function rate_consecutive(&$prev_heat, &$next_heat) {
  $result = 0;
  for ($lane = 0; $lane < count($next_heat); ++$lane) {
    for ($prev_lane = 0; $prev_lane < count($prev_heat); ++$prev_lane) {
      if ($prev_heat[$prev_lane] == $next_heat[$lane]) {
        $result += CONSECUTIVE_WEIGHT;
        if ($lane == $prev_lane) {
          $result += REPEAT_WEIGHT;
        }
      }
    }
  }
  return $result;
}

function rate_next_heat(&$prev_heat, $n_prev_heats, &$next_heat, &$appearances) {
  $rating = rate_race_counts($next_heat, $n_prev_heats, $appearances);
  if ($n_prev_heats > 0) {
    $rating += rate_consecutive($prev_heat, $next_heat);
  }
  return $rating;
}

// Starts with the rough schedule (as produced by generator(s)), and then
// reorders the heats to optimize goals of keeping even race counts, and
// avoiding consecutive races for the same racer.
function make_ordered_schedule($nlanes, $ncars, $n_times_per_lane,
                               $max_runs_per_car, $trace) {
  $rough = make_rough_schedule($nlanes, $ncars, $n_times_per_lane,
                               $max_runs_per_car, $trace);
  if (empty($rough)) {
    return $rough;
  }

  // $appearances[$car] tells how many times $car has appeared in the ordered schedule
  $appearances = array();
  for ($i = 0; $i < $ncars; ++$i) {
    $appearances[] = 0;
  }

  $ordered = array();
  $prev_heat = array();
  for ($round = 0; $round < $n_times_per_lane; ++$round) {
    for ($car_index = 0; $car_index < $ncars; ++$car_index) {
      $best_heat_index = count($rough) - 1;
      $best_heat_score = 10000;
      for ($rough_i = 0; $rough_i < count($rough); ++$rough_i) {
        if ($rough[$rough_i]) {
          $score = rate_next_heat($prev_heat, count($ordered), $rough[$rough_i],
                                  $appearances);
          if ($trace) {
            $trace[] = array('score' => $score, 'index' => $rough_i);
          }
          if ($score < $best_heat_score - 0.00001) {
            $best_heat_score = $score;
            $best_heat_index = $rough_i;
          }
        }
      }

      if ($trace) {
        $trace[] = array('rough-index' => $best_heat_index,
                         'score' => $best_heat_score,
                         'ordered' => $rough[$best_heat_index]);
      }
      
      $prev_heat = $rough[$best_heat_index];
      $rough[$best_heat_index] = NULL;
      foreach ($prev_heat as $car) {
        ++$appearances[$car];
        if ($max_runs_per_car > 0 && $appearances[$car] > $max_runs_per_car) {
          // Exit early, without adding this new heat
          return $ordered;
        }
      }

      $ordered[] = $prev_heat;
    }
  }

  return $ordered;
}

?>