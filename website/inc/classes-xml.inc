<?php
require_once('inc/schema_version.inc');
require_once('inc/classes.inc');

function emit_all_classes() {
  $all_classes = all_classes();
  $classes_by_id = array();
  foreach ($all_classes as $class) {
    $classes_by_id[$class['classid']] = $class;
  }
  foreach ($all_classes as $class) {
    echo '<class classid="'.$class['classid'].'"'
        .' count="'.$class['count'].'"'
        .' nrounds="'.$class['nrounds_with_results'].'"'
        .' ntrophies="'.$class['ntrophies'].'"'
        .' name="'.htmlspecialchars($class['name'], ENT_QUOTES, 'UTF-8').'">'."\n";
    foreach ($class['ranks'] as $rank) {
      echo '  <rank rankid="'.$rank['rankid'].'"'
          .' count="'.$rank['count'].'"'
          .' name="'.htmlspecialchars($rank['name'], ENT_QUOTES, 'UTF-8').'"/>'."\n";
    }
    if (!empty($class['constituents'])) {
      $constituents = explode(',', $class['constituents']);
      foreach ($constituents as $constit_id) {
        echo '  <constituent classid="'.$constit_id.'"'
        .' name="'.htmlspecialchars($classes_by_id[$constit_id]['name'], ENT_QUOTES, 'UTF-8').'"'
        .'/>'."\n";
      }
    }

    echo '</class>'."\n";
  }
}

?>