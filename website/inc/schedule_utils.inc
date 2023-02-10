<?php

// The track may have more lanes than are actually used; unused-lane-mask marks
// the unused lanes.
//
// Accepts as input a race chart of "narrow" heats, and returns a new chart of
// "wide" heats that includes nulls for unused lanes.
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

function results_exist($roundid) {
  global $db;
  return read_single_value('SELECT COUNT(*) FROM RaceChart WHERE roundid = :roundid'
                           .' AND (finishtime IS NOT NULL OR finishplace IS NOT NULL)',
                           array(':roundid' => $roundid));
}

?>
