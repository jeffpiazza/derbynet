<?php

require_once('inc/data.inc');
require_once('inc/newracer.inc');
require_once('inc/schema_version.inc');

// An aggregate round is one which draws from multiple prior rounds.  An
// aggregate class is created by scheduling an aggregate round.  "Grand Finals"
// is the default name for a (first) aggregate class.
//
// Aggregate classes are marked by being in the 'aggregate_class' key in RaceInfo.
// The value of 'aggregate_class' key is a list of classids, separated by '/'
// characters.  (First and last characters of the value are '/'.)

function is_aggregate_class($classid) {
  return schema_version() >= 3 &&
         strlen(read_single_value('SELECT constituents FROM Classes'
                                  .' WHERE classid = :classid',
                                  array(':classid' => $classid))) > 0;
}

// Returns array of {classid, class, constituent, durable, ntrophies} elements
function aggregate_classes() {
  global $db;
  $results = array();
  if (schema_version() >= 3) {
    $stmt = $db->prepare('SELECT classid, class, constituents, durable, ntrophies FROM Classes'
                         .' WHERE constituents > \'\''
                         .' ORDER BY sortorder');
    $stmt->execute(array());
    foreach ($stmt as $cl) {
      $results[] = array('classid' => $cl['classid'],
                         'class' => $cl['class'],
                         'constituents' => explode(',', $cl['constituents']),
                         'durable' => $cl['durable'],
                         'ntrophies' => $cl['ntrophies']);
    }
  }

  return $results;
}

function nonracing_aggregate_classids() {
  global $db;
  $results = array();
  if (schema_version() >= 3) {
    $stmt = $db->prepare('SELECT classid FROM Classes'
                         .' WHERE NOT EXISTS(SELECT 1 FROM Rounds'
                         .'                   WHERE Rounds.classid = Classes.classid)');
    $stmt->execute(array());
    foreach ($stmt as $cl) {
      $results[] = $cl['classid'];
    }
  }
  return $results;
}

function create_aggregate_class($classname, $all_roundids, $durable) {
  global $db;
  $classid = create_class($classname, /* with_first_round= */ false);

  $constituents = array();
  $stmt = $db->prepare('SELECT DISTINCT classid FROM Rounds'
                       .'      WHERE roundid IN ('.$all_roundids.')');
  $stmt->execute(array());
  foreach ($stmt as $row) {
    $constituents[] = $row['classid'];
  }

  $stmt = $db->prepare('UPDATE Classes'
                       .' SET constituents = :constituents,'
                       .' durable = :durable'
                       .' WHERE classid = :classid');
  $stmt->execute(array(':constituents' => implode(',', $constituents),
                       ':durable' => $durable,
                       ':classid' => $classid));
  
  return $classid;
}

// If an aggregate class' only racing round is deleted, the aggregate class gets
// deleted, too, so the name won't conflict with any subsequently-created new
// rounds.
function maybe_clean_up_aggregate_class_for_round($roundid) {
  global $db;
  if (schema_version() < 3) {
    return;
  }

  $classid = read_single_value('SELECT classid FROM Rounds WHERE roundid = :roundid',
                               array(':roundid' => $roundid));

  if (read_single_value('SELECT COUNT(*) FROM Rounds WHERE classid = :classid',
                        array(':classid' => $classid)) > 1) {
    return;
  }

  $stmt = $db->prepare('DELETE FROM Classes'
                       .' WHERE classid = :classid'
                       .' AND constituents > \'\''
                       .' AND NOT durable'
                       .' AND (SELECT COUNT(*) FROM Rounds'
                       .'      WHERE classid = Classes.classid) <= 1');
  $stmt->execute(array(':classid' => $classid));
}
?>