<?php

require_once('inc/data.inc');
require_once('inc/newracer.inc');
require_once('inc/schema_version.inc');
require_once('inc/events.inc');

// An aggregate round is one which draws from multiple prior rounds.  An
// aggregate class is created by scheduling an aggregate round, or by explicit
// user action in the class editor.
//
// When created for an aggregate round, "Grand Finals" is the default name for
// the (first) aggregate class, and the only name for pre-schema-3 databases.

function is_aggregate_class(&$class_row) {
  if (schema_version() >= 3) {
    return strlen($class_row['constituents']) > 0;
  } else {
    return $class_row['class'] == "Grand Finals";
  }
}

// Returns array of {classid, class, constituents, rankids, durable, ntrophies} elements,
// indexed by classid.
function aggregate_classes() {
  global $db;
  $results = array();
  if (schema_version() >= 3) {
    $stmt = $db->prepare('SELECT classid, class, constituents,'
                         .(schema_version() >= 5 ? ' rankids,' : ' \'\' as rankids,')
                         .' durable, ntrophies FROM Classes'
                         .' WHERE constituents > \'\''
                         .' ORDER BY sortorder');
  } else {
    $stmt = $db->prepare('SELECT classid, class, \'\' AS constituents,'
                         .' \'\' as rankids, 0 AS durable,'
                         .'   ntrophies FROM Classes'
                         .' WHERE class = \'Grand Finals\''
                         .' ORDER BY sortorder');
  }

  $stmt->execute(array());
  foreach ($stmt as $cl) {
    $results[$cl['classid']] = array('classid' => $cl['classid'],
                                     'class' => $cl['class'],
                                     'constituents' =>
                                         array_filter(explode(',', $cl['constituents']), 'strlen'),
                                     'rankids' =>
                                         array_filter(explode(',', $cl['rankids']), 'strlen'),
                                     'durable' => $cl['durable'],
                                     'ntrophies' => $cl['ntrophies']);
  }

  return $results;
}

// Updates $agg_classes argument, which is a map of aggregate classes as
// returned by aggregate_classes(), and adds ['constituents_tc'] to each
// element, containing the transitive closure of the constituents relation.
function compute_aggregate_constituents_tc(&$agg_classes) {
  foreach ($agg_classes as &$cl) {
    $cl['constituents_tc'] = $cl['constituents'];
  }

  do {
    $repeat = false;
    foreach ($agg_classes as &$cl) {
      foreach ($cl['constituents_tc'] as $constituent_classid) {
        if (isset($agg_classes[$constituent_classid])) {
          foreach ($agg_classes[$constituent_classid]['constituents'] as $second_classid) {
            if (!in_array($second_classid, $cl['constituents_tc'])) {
              $cl['constituents_tc'][] = $second_classid;
              $repeat = true;
            }
          }
        }
      }
    }
  } while ($repeat);

  return $agg_classes;
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

function create_aggregate_class($classname, $constituents, $rankids, $durable) {
  global $db;
  $classid = create_class($classname, /* with_first_round= */ false);

  if (is_array($constituents)) {
    $constituents = implode(',', $constituents);
  }
  if (is_array($rankids)) {
    $rankids = implode(',', $rankids);
  }

  $stmt = $db->prepare('UPDATE Classes'
                       .' SET constituents = :constituents,'
                       .' rankids = :rankids,'
                       .' durable = :durable'
                       .' WHERE classid = :classid');
  $stmt->execute(array(':constituents' => $constituents,
                       ':rankids' => $rankids,
                       ':durable' => $durable,
                       ':classid' => $classid));

  $stmt = $db->prepare('INSERT INTO ConstituentClasses(antecedent, dependent) VALUES(:ante, :dep)');
  foreach (explode(',', $constituents) as $ante) {
    $stmt->execute(array(':ante' => $ante, ':dep' => $classid));
  }
  
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
  if ($stmt->rowCount() > 0) {
    record_event(EVENT_CLASS_DELETED, array('classid' => $classid));
    execute_once('DELETE FROM ConstituentClasses WHERE dependent = :classid',
                 array(':classid' => $classid));
  }

  // Clean up any playlist entries for the deleted class.  When activated, a
  // playlist entry for a nonexistent class will create a new round for the
  // class, which can lead to some confusion.  (This actually removes Playlist
  // entries for any non-existent class, not just $classid.)
  $stmt = $db->prepare('DELETE FROM Playlist'
                       .' WHERE NOT EXISTS('
                       .    'SELECT 1 FROM Classes'
                       .    ' WHERE classid = Playlist.classid'
                       .')');
  $stmt->execute(array());
}
?>