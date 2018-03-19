<?php
// Action to schedule heats for a round.
// $_POST: roundid, nrounds(=1)
// nrounds here is the number of times each racer should race in each lane.

// Add tests for no (checked-in) racers, one, two, ..., nlanes-1

// Action to set number of lanes (by interrogating timer)


require_once('inc/data.inc');
require_once('inc/masterheat.inc');

// Each generator is a simple array of ints.
// $generators[/* number of lanes */] gives an array of entries; each
// entry has min and max number of cars plus an array of generators.

function make_entry($min, $max, $generators) {
  return array('min' => $min,
               'max' => $max,
               'generators' => $generators);
}

$all_generators = array(
  1 => array(make_entry(1, 200, array(array(), array(), array(), array(), array(), array()))),
  2 => array(make_entry(2,2, array(array(1), array(1))),
             make_entry(3,3, array(array(2), array(1))),
             make_entry(4,4, array(array(3), array(2))),
             make_entry(5,5, array(array(3), array(4), array(2), array(1))),
             make_entry(6,6, array(array(2), array(5))),
             make_entry(7,7, array(array(3), array(2), array(1), array(4),
                                   array(5), array(6))),
             make_entry(8,8, array(array(3), array(2), array(1))),
             make_entry(9,9, array(array(4), array(3), array(2), array(1),
                                   array(5), array(6), array(7), array(8))),
             make_entry(10,10, array(array(4), array(3), array(2), array(1))),
             make_entry(11,11, array(array(5), array(4), array(3), array(2),
                                     array(1), array(6), array(7), array(8),
                                     array(9), array(10))),
             make_entry(12,12, array(array(5), array(4), array(3), array(2),
                                     array(1))),
             make_entry(13,13, array(array(6), array(5), array(4), array(3),
                                     array(2), array(1), array(7), array(8),
                                     array(9), array(10), array(11), array(12))),
             make_entry(14,14, array(array(6), array(5), array(4), array(3),
                                     array(2), array(1))),
             make_entry(15,15, array(array(7), array(6), array(5), array(4),
                                     array(3), array(2), array(1), array(8),
                                     array(9), array(10), array(11), array(12))),
             make_entry(16,16, array(array(7), array(6), array(5), array(4),
                                     array(3), array(2), array(1))),
             make_entry(17,17, array(array(8), array(7), array(6), array(5),
                                     array(4), array(3), array(2), array(1),
                                     array(9), array(10), array(11), array(12))),
             make_entry(18,18, array(array(8), array(7), array(6), array(5),
                                     array(4), array(3), array(2), array(1))),
             make_entry(19,19, array(array(9), array(8), array(7), array(6),
                                     array(5), array(4), array(3), array(2),
                                     array(1), array(10), array(11), array(12))),
             make_entry(20,20, array(array(9), array(8), array(7), array(6),
                                     array(5), array(4), array(3), array(2),
                                     array(1))),
             make_entry(21,21, array(array(10), array(9), array(8), array(7),
                                     array(6), array(5), array(4), array(3),
                                     array(2), array(1), array(11), array(12))),
             make_entry(22,22, array(array(10), array(9), array(8), array(7),
                                     array(6), array(5), array(4), array(3),
                                     array(2), array(1))),
             make_entry(23,23, array(array(11), array(10), array(9), array(8),
                                     array(7), array(6), array(5), array(4),
                                     array(3), array(2), array(1), array(12))),
             make_entry(24,24, array(array(11), array(10), array(9), array(8),
                                     array(7), array(6), array(5), array(4),
                                     array(3), array(2), array(1))),
             make_entry(25,200, array(array(12), array(11), array(10), array(9),
                                      array(8), array(7), array(6), array(5),
                                      array(4), array(3), array(2), array(1)))
    ),
  3 => array(
    make_entry(3,3, array(array(2, 2), array(1, 1))),
    make_entry(4,4, array(array(3, 3), array(1, 1))),
    make_entry(5,5, array(array(2, 2), array(1, 1), array(3, 3), array(4, 4))),
    make_entry(6,6, array(array(2, 3), array(5, 2))),
    make_entry(7,7, array(array(2, 4), array(5, 3))),
    make_entry(8,8, array(array(2, 5), array(3, 4))),
    make_entry(9,9, array(array(2, 3), array(3, 5))),
    make_entry(10,10, array(array(2, 7), array(4, 5))),
    make_entry(11,11, array(array(2, 3), array(3, 7))),
    make_entry(12,12, array(array(2, 3))),
    make_entry(13,13, array(array(3, 9), array(7, 11), array(10, 4), array(6, 2))),
    make_entry(14,14, array(array(2, 3))),
    make_entry(15,15, array(array(2, 3), array(6, 8))),
    make_entry(16,16, array(array(2, 3), array(6, 9))),
    make_entry(17,18, array(array(2, 3), array(4, 6))),
    make_entry(19,19, array(array(2, 3), array(4, 6), array(1, 7), array(17, 16))),
    make_entry(20,20, array(array(2, 3), array(4, 7))),
    make_entry(21,200, array(array(2, 3), array(4, 6)))
    ),
  4 => array(
    make_entry(4,4, array(array(3, 3, 3), array(1, 1, 1))),
    make_entry(5,5, array(array(2, 2, 2), array(3, 3, 3))),
    make_entry(6,6, array(array(2, 2, 3), array(3, 5, 5))),
    make_entry(7,7, array(array(2, 2, 4), array(5, 5, 3))),
    make_entry(8,8, array(array(2, 2, 3), array(3, 4, 2))),
    make_entry(9,9, array(array(2, 2, 4), array(3, 5, 3))),
    make_entry(10,10, array(array(2, 2, 5), array(3, 3, 6))),
    make_entry(11,11, array(array(2, 2, 6), array(3, 3, 4))),
    make_entry(12,12, array(array(2, 4, 5), array(3, 2, 8))),
    make_entry(13,13, array(array(2, 4, 12), array(11, 9, 1))),
    make_entry(14,14, array(array(2, 4, 13), array(3, 5, 2))),
    make_entry(15,15, array(array(2, 3, 4), array(3, 2, 9))),
    make_entry(16,16, array(array(2, 3, 7), array(3, 5, 9))),
    make_entry(17,17, array(array(2, 3, 4), array(3, 2, 11))),
    make_entry(18,18, array(array(2, 3, 7), array(3, 5, 9))),
    make_entry(19,19, array(array(2, 3, 4), array(3, 5, 13))),
    make_entry(20,20, array(array(2, 14, 11), array(12, 18, 3))),
    make_entry(21,21, array(array(4, 5, 10), array(5, 13, 7))),
    make_entry(22,22, array(array(4, 5, 7), array(8, 12, 21))),
    make_entry(23,23, array(array(4, 7, 10), array(3, 5, 14))),
    make_entry(24,26, array(array(2, 3, 4))),
    make_entry(27,27, array(array(4, 5, 6), array(7, 19, 25))),
    make_entry(28,28, array(array(4, 5, 6), array(7, 20, 26))),
    make_entry(29,29, array(array(2, 3, 4), array(6, 11, 28))),
    make_entry(30,30, array(array(2, 3, 4), array(6, 11, 29))),
    make_entry(31,31, array(array(2, 3, 4), array(6, 11, 30))),
    make_entry(32,32, array(array(2, 3, 4), array(6, 12, 31))),
    make_entry(33,33, array(array(2, 3, 4), array(6, 12, 32))),
    make_entry(34,34, array(array(2, 3, 4), array(6, 13, 33))),
    make_entry(35,35, array(array(2, 3, 4), array(6, 8, 10))),
    make_entry(36,36, array(array(2, 3, 4), array(6, 8, 12))),
    make_entry(37,37, array(array(2, 3, 4), array(6, 8, 10))),
    make_entry(38,38, array(array(2, 3, 4), array(6, 8, 13))),
    make_entry(39,41, array(array(2, 3, 4), array(6, 8, 10))),
    make_entry(42,42, array(array(2, 3, 4), array(6, 8, 11))),
    make_entry(43,47, array(array(2, 3, 4), array(6, 8, 10))),
    make_entry(48,48, array(array(2, 3, 4), array(6, 8, 11))),
    make_entry(49,200, array(array(2, 3, 4), array(6, 8, 10))),
    ),
  5 => array(
    make_entry(5,5, array(array(2, 2, 2, 2), array(3, 3, 3, 3), 
                          array(1, 1, 1, 1), array(4, 4, 4, 0))),
    make_entry(6,6, array(array(5, 5, 5, 5), array(1, 1, 1, 1))),
    make_entry(7,7, array(array(2, 2, 2, 2), array(3, 3, 3, 3))),
    make_entry(8,8, array(array(2, 2, 3, 2), array(3, 3, 4, 5))),
    make_entry(9,9, array(array(2, 2, 2, 4), array(3, 3, 5, 8))),
    make_entry(10,10, array(array(2, 2, 3, 4), array(3, 3, 6, 9))),
    make_entry(11,11, array(array(2, 2, 3, 5), array(9, 9, 8, 6))),
    make_entry(12,12, array(array(2, 2, 3, 6), array(3, 3, 2, 8))),
    make_entry(13,13, array(array(2, 2, 3, 7), array(3, 3, 2, 4))),
    make_entry(14,14, array(array(2, 2, 3, 6))),
    make_entry(15,15, array(array(2, 2, 3, 9), array(3, 4, 2, 5))),
    make_entry(16,16, array(array(2, 2, 4, 7), array(3, 3, 6, 5))),
    make_entry(17,17, array(array(2, 2, 4, 16), array(3, 3, 2, 8))),
    make_entry(18,18, array(array(2, 3, 4, 8), array(3, 2, 2, 10))),
    make_entry(19,19, array(array(2, 2, 7, 13), array(3, 3, 2, 10))),
    make_entry(20,20, array(array(2, 2, 3, 7), array(3, 3, 2, 4))),
    make_entry(21,21, array(array(2, 5, 4, 18), array(19, 16, 17, 3))),
    make_entry(22,22, array(array(2, 2, 3, 7), array(3, 3, 2, 4))),
    make_entry(23,23, array(array(2, 3, 8, 16), array(3, 2, 7, 10))),
    make_entry(24,24, array(array(2, 5, 4, 21), array(3, 2, 6, 4))),
    make_entry(25,25, array(array(2, 3, 4, 10), array(3, 2, 7, 12))),
    make_entry(26,26, array(array(2, 3, 7, 25), array(3, 2, 4, 4))),
    make_entry(27,27, array(array(2, 3, 4, 6), array(3, 2, 6, 20))),
    make_entry(28,28, array(array(2, 3, 4, 8), array(3, 2, 8, 14))),
    make_entry(29,29, array(array(2, 3, 4, 6), array(3, 2, 7, 28))),
    make_entry(30,30, array(array(2, 3, 4, 10), array(3, 4, 8, 16))),
    make_entry(31,31, array(array(2, 3, 4, 6), array(3, 2, 18, 30))),
    make_entry(32,32, array(array(1, 2, 4, 5), array(2, 6, 7, 3))),
    make_entry(33,33, array(array(2, 3, 4, 6), array(3, 5, 12, 32))),
    make_entry(34,34, array(array(2, 3, 4, 6), array(3, 5, 12, 33))),
    make_entry(35,35, array(array(1, 2, 4, 5), array(2, 6, 10, 4))),
    make_entry(36,49, array(array(2, 3, 4, 6))),
    make_entry(50,50, array(array(2, 3, 4, 6), array(8, 11, 17, 34))),
    make_entry(51,51, array(array(2, 3, 4, 6), array(8, 11, 18, 34))),
    make_entry(52,52, array(array(2, 3, 4, 6))),
    make_entry(53,53, array(array(2, 3, 4, 6), array(8, 11, 18, 36))),
    make_entry(54,54, array(array(2, 3, 4, 6), array(8, 11, 17, 38))),
    make_entry(55,55, array(array(2, 3, 4, 6), array(8, 12, 17, 54))),
    make_entry(56,56, array(array(2, 3, 4, 6), array(8, 11, 21, 55))),
    make_entry(57,57, array(array(2, 3, 4, 6), array(8, 11, 17, 41))),
    make_entry(58,58, array(array(2, 3, 4, 6), array(8, 11, 17, 42))),
    make_entry(59,59, array(array(2, 3, 4, 6), array(8, 11, 17, 43))),
    make_entry(60,60, array(array(2, 3, 4, 6), array(8, 11, 17, 44))),
    make_entry(61,160, array(array(2, 3, 4, 6))),
    make_entry(161,200, array(array(2, 3, 4, 6), array(8, 11, 17, 44)))
    ),
  6 => array(
    make_entry(6,6, array(array(5, 5, 5, 5, 5), array(1, 1, 1, 1, 1))),
    make_entry(7,7, array(array(2, 2, 2, 2, 2), array(5, 5, 5, 5, 5))),
    make_entry(8,8, array(array(2, 2, 2, 3, 2))),
    make_entry(9,9, array(array(2, 2, 2, 2, 4))),
    make_entry(10,10, array(array(2, 2, 2, 3, 2))),
    make_entry(11,11, array(array(2, 2, 3, 5, 4))),
    make_entry(12,12, array(array(2, 2, 2, 3, 4))),
    make_entry(13,13, array(array(2, 2, 2, 3, 5))),
    make_entry(14,14, array(array(2, 2, 2, 3, 6))),
    make_entry(15,15, array(array(2, 2, 2, 3, 7))),
    make_entry(16,16, array(array(1, 2, 2, 3, 6))),
    make_entry(17,17, array(array(1, 2, 2, 3, 6))),
    make_entry(18,18, array(array(2, 2, 3, 5, 9))),
    make_entry(19,19, array(array(2, 2, 3, 4, 9))),
    make_entry(20,20, array(array(2, 2, 3, 4, 19))),
    make_entry(21,21, array(array(2, 2, 3, 4, 20))),
    make_entry(22,22, array(array(2, 2, 3, 4, 21))),
    make_entry(23,23, array(array(2, 2, 3, 4, 22))),
    make_entry(24,24, array(array(2, 2, 3, 6, 23))),
    make_entry(25,25, array(array(2, 2, 3, 6, 24))),
    make_entry(26,26, array(array(2, 2, 3, 6, 25))),
    make_entry(27,27, array(array(2, 2, 3, 6, 26))),
    make_entry(28,28, array(array(2, 5, 11, 4, 27))),
    make_entry(29,29, array(array(2, 3, 7, 15, 17))),
    make_entry(30,30, array(array(2, 3, 7, 15, 17))),
    make_entry(31,31, array(array(2, 3, 7, 15, 17))),
    make_entry(32,32, array(array(4, 1, 2, 8, 16))),
    make_entry(33,33, array(array(4, 1, 2, 8, 16))),
    make_entry(34,34, array(array(4, 1, 2, 8, 16))),
    make_entry(35,35, array(array(2, 3, 7, 19, 17))),
    make_entry(36,36, array(array(2, 3, 7, 20, 17))),
    make_entry(37,37, array(array(2, 3, 4, 10, 12))),
    make_entry(38,38, array(array(2, 3, 4, 8, 10))),
    make_entry(39,39, array(array(2, 3, 4, 19, 38))),
    make_entry(40,40, array(array(2, 3, 4, 6, 8))),
    make_entry(41,41, array(array(2, 3, 4, 6, 12))),
    make_entry(42,42, array(array(2, 3, 4, 8, 14))),
    make_entry(43,43, array(array(2, 3, 4, 6, 8))),
    make_entry(44,44, array(array(2, 3, 4, 6, 11))),
    make_entry(45,45, array(array(2, 3, 4, 6, 8))),
    make_entry(46,46, array(array(2, 3, 4, 6, 11))),
    make_entry(47,200, array(array(2, 3, 4, 6, 8)))
    )
  );

// Returns an array of generators, appropriate for $nlanes and $ncars.
function get_generators($nlanes, $ncars) {
  global $all_generators;
  foreach ($all_generators[$nlanes] as $entry) {
    if ($entry['min'] <= $ncars && $ncars <= $entry['max']) {
      return $entry['generators'];
    }
  }
}

// The rough schedule is an array of heats.  Each heat is an array of length
// $nlanes, each entry being a value c, 0 <= c < $ncars.  (c is not a racerid.)
function make_rough_schedule($nlanes, $ncars, $nrounds) {
  $generators = get_generators($nlanes, $ncars);
  if ($nrounds > count($generators)) {
    $nrounds = count($generators);
  }

  $heats = array();
  for ($round = 0; $round < $nrounds; ++$round) {
    $gen = $generators[$round];
    for ($h = 0; $h < $ncars; ++$h) {
      $heat = array();
      $heat[] = $h;
      for ($lane = 1; $lane < $nlanes; ++$lane) {
        $heat[] = ($heat[$lane - 1] + $gen[$lane - 1]) % $ncars;
      }
      $heats[] = $heat;
    }
  }
  // echo "<rough-schedule>\n"; var_dump($heats); echo "</rough-schedule>\n";

  return $heats;
}

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
  if ($n_prev_heats == 0) {
    return rate_race_counts($next_heat, $n_prev_heats, $appearances);
  } else {
    return rate_race_counts($next_heat, $n_prev_heats, $appearances) + rate_consecutive($prev_heat, $next_heat);
  }
}

// Starts with the rough schedule (as produced by generator(s)), and then
// reorders the heats to optimize goals of keeping even race counts, and
// avoiding consecutive races for the same racer.
function make_ordered_schedule($nlanes, $ncars, $nrounds) {
  $rough = make_rough_schedule($nlanes, $ncars, $nrounds);
  $appearances = array();
  for ($i = 0; $i < $ncars; ++$i) {
    $appearances[] = 0;
  }

  $ordered = array();
  $prev_heat = array();
  for ($round = 0; $round < $nrounds; ++$round) {
    for ($car_index = 0; $car_index < $ncars; ++$car_index) {
      $best_heat_index = count($rough) - 1;
      $best_heat_score = 10000;
      for ($rough_i = 0; $rough_i < count($rough); ++$rough_i) {
        if ($rough[$rough_i]) {
          $score = rate_next_heat($prev_heat, count($ordered), $rough[$rough_i], $appearances);
          if ($score < $best_heat_score - 0.00001) {
            $best_heat_score = $score;
            $best_heat_index = $rough_i;
          }
        }
      }

      $prev_heat = $rough[$best_heat_index];
      $rough[$best_heat_index] = NULL;
      foreach ($prev_heat as $car) {
        ++$appearances[$car];
      }
      $ordered[] = $prev_heat;
    }
  }

  return $ordered;
}

// Returns roster, where roster[sequence] = racerid,
// 0 <= sequence < number of racers in roundid.
function read_roster($roundid) {
  global $db;
  $roster = array();
  $stmt = $db->prepare('SELECT rosterid, Roster.racerid FROM Roster'
                       .' INNER JOIN RegistrationInfo'
                       .' ON Roster.racerid = RegistrationInfo.racerid'
                       .' WHERE roundid = :roundid'
                       .' AND passedinspection = 1'
                       .' ORDER BY rosterid');
  $stmt->execute(array(':roundid' => $roundid));
  foreach ($stmt as $row) {
    $roster[] = $row['racerid'];
  }
  return $roster;
}

// Converts a schedule (racers identified by sequence, 0 <= c < $ncars) to a
// race chart (racers identified by racerid).
function form_chart(&$schedule, &$roster) {
  $heats = array();
  foreach ($schedule as $sched_heat) {
    $heat = array();
    foreach ($sched_heat as $index) {
      $heat[] = $roster[$index];
    }
    $heats[] = $heat;
  }
  return $heats;
}

// The track may have more lanes than are actually used.
function spread_across_unused_lanes(&$chart) {
  // 1's in the mask mark lanes that will NOT be used.
  $mask = 0 + read_raceinfo('unused-lane-mask', 0);
  if ($mask == 0) {
    return $chart;
  }
  $wide = array();
  foreach ($chart as $narrow_heat) {
    $p = 0;  // Physical lane index
    $wide_heat = array();
    foreach ($narrow_heat as $racer) {
      while (($mask & (1 << $p)) != 0) {
        $wide_heat[] = NULL;
        ++$p;
      }
      $wide_heat[] = $racer;
      ++$p;
    }
    $wide[] = $wide_heat;
  }
  return $wide;
}

// Writes the race chart entries into the database.
function write_chart($roundid, &$heats) {
  global $db;
  $db->exec('DELETE FROM RaceChart WHERE roundid = '.$roundid);
  $classid = read_single_value('SELECT classid FROM Rounds WHERE roundid = '.$roundid);
  $stmt = $db->prepare('INSERT INTO RaceChart(roundid, heat, lane, racerid, classid)'
                       .' VALUES(:roundid, :heat, :lane, :racerid, :classid)');
  $heat_index = 0;
  foreach ($heats as $heat) {
    ++$heat_index;
    $lane_index = 0;
    foreach ($heat as $racerid) {
      ++$lane_index;
      if ($racerid) {
        $stmt->execute(array(':roundid' => $roundid,
                             ':heat' => $heat_index,
                             ':lane' => $lane_index,
                             ':racerid' => $racerid,
                             ':classid' => $classid));
      }
    }
  }
}

function results_exist($roundid) {
  global $db;
  return read_single_value('SELECT COUNT(*) FROM RaceChart WHERE roundid = :roundid AND finishtime IS NOT NULL',
                           array(':roundid' => $roundid));
}


// The top-level scheduling function.  Constructs a schedule, converts to a race
// chart, and writes the race chart into the database.
function schedule_one_round($roundid, $nlanes, $nrounds = 1) {
  $roster = read_roster($roundid);
  if (count($roster) < 2) {
    return false;
  }
  while (count($roster) < $nlanes) {
    $roster[] = null;
  }
  $sched = make_ordered_schedule($nlanes, count($roster), $nrounds);
  write_chart($roundid, spread_across_unused_lanes(form_chart($sched, $roster)));
  return true;
}

// For debugging only:
function print_chart(&$heats) {
  echo '<table>';
  $first = true;
  foreach ($heats as $heat) {
    if ($first) {
      echo '<tr>';
      for ($lane = 1; $lane <= count($heat); ++$lane) {
        echo '<th>Lane '.$lane.'</th>';
      }
      echo '</tr>'."\n";
      $first = false;
    }
    echo '<tr>';
    foreach ($heat as $car) {
      echo '<td>'.$car.'</td>';
    }
    echo '</tr>'."\n";
  }
  echo '</table>'."\n";
}

function read_race_chart($roundid) {
  global $db;
  $heats = array();
  $stmt = $db->prepare('SELECT heat, lane, racerid FROM RaceChart WHERE roundid = :roundid ORDER BY heat, lane');
  foreach ($stmt->execute(array(':roundid' => $roundid)) as $row) {
    while ($row['heat'] > count($heats)) {
      $heats[] = array();
    }
    while ($row['lane'] > count($heats[$row['heat'] - 1])) {
      $heats[$row['heat'] - 1][] = 'Bye';
    }
    $heats[$row['heat'] - 1][$row['lane'] - 1] = $row['racerid'];
  }

  return $heats;
}

function count_one_bits($n) {
  $onebits = 0;
  while ($n != 0) {
    if (($n & 1) != 0) {
      ++$onebits;
    }
    $n = $n >> 1;
  }
  return $onebits;
}

start_response();

if (have_permission(CONTROL_RACE_PERMISSION)) {
  // $nlanes reports the number of physical lanes, even if some won't be used.
  $nlanes = read_raceinfo('lane_count', 0);
  // For generating the schedule, we care about the number of lanes actually in use.
  $nlanes -= count_one_bits(read_raceinfo('unused-lane-mask', 0));
  if ($nlanes <= 0) {
	echo '<failure code="nlanes">Number of lanes hasn\'t been determined.</failure>';
  } else if (!isset($all_generators[$nlanes])) {
    echo '<failure code="too-many-lanes">Lane scheduling is only available for up to '
    .max(array_keys($all_generators)).' lanes.</failure>';
  } else if (results_exist($_POST['roundid'])) {
	echo '<failure code="results-exist">Race results already exist for this round</failure>';
  } else {
	$nrounds = isset($_POST['nrounds']) ? $_POST['nrounds'] : 1;
	if (schedule_one_round($_POST['roundid'], $nlanes, $nrounds)) {
      echo '<success/>';
      require('ajax/query.poll.coordinator.inc');
    } else {
      echo '<failure code="too-few-racers">There need to be at least two racers checked in'
      .' to generate a schedule.</failure>';
    }
  }

  assign_master_heats(read_single_value('SELECT round FROM Rounds WHERE roundid = :roundid',
                                        array(':roundid' => $_POST['roundid'])));
} else {
  not_authorized_body();
}

end_response();
?>
