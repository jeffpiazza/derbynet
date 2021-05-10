<?php

require_once('inc/schema_version.inc');
require_once('inc/classes.inc');

function json_all_classes() {
  $all_classes = all_classes();

  $classes_by_id = array();
  foreach ($all_classes as $class) {
    $classes_by_id[$class['classid']] = $class;
  }
  $result = array();

  foreach ($all_classes as $class) {
    $cl = array(
      'classid' => $class['classid'] + 0,
      'nrounds' => $class['nrounds_with_results'] + 0,
      'ntrophies' => $class['ntrophies'] + 0,
      'name' => $class['name']);
    if (!empty($class['rankids'])) {
      $cl['aggregate-by-subgroup'] = '1';
    }
    $cl['subgroups'] = array();
    foreach ($class['ranks'] as $rank) {
      $cl['subgroups'][] = array(
        'rankid' => $rank['rankid'] + 0,
        'count' => $rank['count'] + 0,
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