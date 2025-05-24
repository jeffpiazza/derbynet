<?php

require_once('inc/schema_version.inc');
require_once('inc/classes.inc');

// [ {classid, count, nrounds, ntrophies, name, aggregate-by-subgroup
//    subgroups: [{rankid, name, count, ntrophies}...],
//    constituents: [classid, classid, ...] }, ... ]

function json_all_classes() {
  $all_classes = all_classes();

  $classes_by_id = array();
  $ranks_by_id = array();
  foreach ($all_classes as $class) {
    $classes_by_id[$class['classid']] = $class;
    foreach ($class['ranks'] as $rank) {
      $ranks_by_id[$rank['rankid']] = $rank;
    }
  }

  $result = array();

  foreach ($all_classes as $class) {
    $cl = array(
      'classid' => $class['classid'],
      'count' => $class['count'],
      'nrounds' => $class['nrounds_with_results'],
      'ntrophies' => $class['ntrophies'],
      'name' => $class['name']);
    if (!empty($class['rankids'])) {
      $cl['aggregate-by-subgroup'] = true;
    }
    $cl['subgroups'] = array();
    foreach ($class['ranks'] as $rank) {
      $cl['subgroups'][] = array(
        'rankid' => $rank['rankid'],
        'count' => $rank['count'],
        'ntrophies' => $rank['ntrophies'],
        'name' => $rank['name']);
    }
    if (!empty($class['constituents'])) {
      $constituents = explode(',', $class['constituents']);
      $cl['constituents'] = array();
      foreach ($constituents as $constit_id) {
        $cl['constituents'][] = array(
          'classid' => $constit_id,
          'name' => $classes_by_id[$constit_id]['name']);
      }
    }
    if (!empty($class['rankids'])) {
      $rankids = explode(',', $class['rankids']);
      list($classes, $classseq, $ranks, $rankseq) = classes_and_ranks();
      $cl['constituent-ranks'] = array();
      foreach ($rankids as $r) {
        $rr = $ranks_by_id[$r];
        $cl['constituent-ranks'][] = array(
          'rankid' => $r,
          'name' => $rr['name'],
          'classid' => $ranks[$r]['classid'],
          'class' => $ranks[$r]['class']);
      }
    }
    $result[] = $cl;
  }
  return $result;
}
?>
