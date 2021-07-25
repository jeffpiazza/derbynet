<?php

require_once('inc/schema_version.inc');
require_once('inc/classes.inc');

// [ {classid, count, nrounds, ntrophies, name, aggregate-by-subgroup
//    subgroups: [{rankid, name, count}...],
//    constituents: [classid, classid, ...] }, ... ]

function json_all_classes() {
  $all_classes = all_classes();

  $classes_by_id = array();
  foreach ($all_classes as $class) {
    $classes_by_id[$class['classid']] = $class;
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
    $result[] = $cl;
  }
  return $result;
}
?>