<?php

require_once('inc/data.inc');
require_once('inc/newracer.inc');

// An aggregate round is one which draws from multiple prior rounds.  An
// aggregate class is created by scheduling an aggregate round.  "Grand Finals"
// is the default name for a (first) aggregate class.
//
// Aggregate classes are marked by being in the 'aggregate_class' key in RaceInfo.
// The value of 'aggregate_class' key is a list of classids, separated by '/'
// characters.  (First and last characters of the value are '/'.)

function is_aggregate_class($classid) {
  return strpos(read_raceinfo('aggregate-classes', '/'), '/'.$classid.'/') !== false;
}

// Returns array of {classid, class} elements
function aggregate_classes() {
  global $db;
  $agg_classes = read_raceinfo('aggregate-classes', '/');
  $results = array();
  if ($agg_classes !== false) {
    $stmt = $db->prepare('SELECT classid, class FROM Classes'
                         .' WHERE classid IN (0'.str_replace('/', ',', $agg_classes).'0)');
    $stmt->execute(array());
    foreach ($stmt as $cl) {
      $results[] = array('classid' => $cl['classid'],
                         'class' => $cl['class']);
    }
  }

  return $results;
}

// Returns an array mapping classid to trophy count for that class.  If a
// classid doesn't appear in the array, it should should the default number of
// trophies per class.
//
// 'trophies-per-class' is a sequence of <classid1>,<count1>,<classid2>,<count2>,...
function trophies_per_class() {
  $entries = explode(',', read_raceinfo('trophies-per-class', ''));
  $result = array();
  for ($i = 0; $i < count($entries); $i += 2) {
    $result[$entries[$i]] = $entries[$i + 1];
  }
  return $result;
}

function create_aggregate_class($classname) {
  $classid = create_class($classname, /* with_first_round= */ false);

  $agg_keys = read_raceinfo('aggregate-classes', '/');
  write_raceinfo('aggregate-classes', $agg_keys.$classid.'/');
  
  return $classid;
}

// If an aggregate class' only racing round is deleted, the aggregate class gets
// deleted, too, so the name won't conflict with any subsequently-created new
// rounds.
function maybe_clean_up_aggregate_class_for_round($roundid) {
  global $db;
  $classid = read_single_value('SELECT classid FROM Rounds WHERE roundid = :roundid',
                               array(':roundid' => $roundid));

  if (read_single_value('SELECT COUNT(*) FROM Rounds WHERE classid = :classid',
                        array(':classid' => $classid)) > 1) {
    return;
  }
  
  $agg_classes = read_raceinfo('aggregate-classes', '/');
  $pos = strpos($agg_classes, '/'.$classid.'/');
  if ($pos !== false) {
    $stmt = $db->prepare('DELETE FROM Classes WHERE classid = :classid');
    $stmt->execute(array(':classid' => $classid));

    write_raceinfo('aggregate-classes',
                   substr($agg_classes, 0, $pos).substr($agg_classes, $pos + strlen($classid.'/')));
  }
}
?>