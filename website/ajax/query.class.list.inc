<classes_and_ranks>
<?php
require_once('inc/schema_version.inc');
require_once('inc/classes.inc');

foreach (all_classes() as $class) {
  echo '<class classid="'.$class['classid'].'"'
            .' count="'.$class['count'].'"'
            .' nrounds="'.$class['nrounds_with_results'].'"'
            .' name="'.htmlspecialchars($class['name'], ENT_QUOTES, 'UTF-8').'">'."\n";
  foreach ($class['ranks'] as $rank) {
    echo '  <rank rankid="'.$rank['rankid'].'"'
        .' count="'.$rank['count'].'"'
        .' name="'.htmlspecialchars($rank['name'], ENT_QUOTES, 'UTF-8').'"/>'."\n";
  }
  echo '</class>'."\n";
}

?>
</classes_and_ranks>