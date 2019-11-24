<?php
require_once('inc/schema_version.inc');
require_once('inc/classes.inc');

function emit_all_classes() {
  foreach (all_classes() as $class) {
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
    echo '</class>'."\n";
  }
}

?>