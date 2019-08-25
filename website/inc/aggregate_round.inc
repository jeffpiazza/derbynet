<?php

require_once('inc/data.inc');
require_once('inc/newracer.inc');

function is_aggregate_class($classid) {
  return strpos(read_raceinfo('aggregate_classes', '/'), '/'.$classid.'/') !== false;
}

function aggregate_classes() {
  $agg_classes = substr(read_raceinfo('aggregate_classes', '/'), 1, -1);
  if ($agg_classes === false) {
    return array();
  }

  return explode('/', $agg_classes);
}

function create_aggregate_class($classname) {
  $classid = create_class($classname, /* with_first_round= */ false);

  $agg_keys = read_raceinfo('aggregate_classes', '/');
  write_raceinfo('aggregate_classes', $agg_keys.$classid.'/');
  
  return $classid;
}

function maybe_clean_up_aggregate_class_for_round($roundid) {
  global $db;
  $classid = read_single_value('SELECT classid FROM Rounds WHERE roundid = :roundid',
                               array(':roundid' => $roundid));

  if (read_single_value('SELECT COUNT(*) FROM Rounds WHERE classid = :classid',
                        array(':classid' => $classid)) > 1) {
    return;
  }
  
  $agg_classes = read_raceinfo('aggregate_classes', '/');
  $pos = strpos($agg_classes, '/'.$classid.'/');
  if ($pos !== false) {
    $stmt = $db->prepare('DELETE FROM Classes WHERE classid = :classid');
    $stmt->execute(array(':classid' => $classid));

    write_raceinfo('aggregate_classes',
                   substr($agg_classes, 0, $pos).substr($agg_classes, $pos + strlen($classid.'/')));
  }
}

?>