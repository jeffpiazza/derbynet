<?php

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
  // If there's a non-zero max-runs-per-car value, then we adopt a simple rotation schedule,
  // which means a generator of 1 for each lane.
  if (read_raceinfo("max-runs-per-car", 0) != 0) {
    $generators = array();
    for ($i = 0; $i < $nlanes - 1; ++$i) {
      $generators[] = 1;
    }
    return array($generators);
  }
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
  if (false) {  // For debugging:
    echo "<rough-schedule>\n";
    $hi = 0;
    foreach ($heats as $h) {
      ++$hi;
      echo "<heat heat='".$hi."'>".json_encode($h)."</heat>\n";
    }
    echo "</rough-schedule>\n";
  }

  return $heats;
}

?>