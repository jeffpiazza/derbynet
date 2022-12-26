<?php

require_once('inc/schema_version.inc');
require_once('inc/sql-script.inc');
require_once(sql_file_path('column-widths'));
require_once('inc/events.inc');
require_once('inc/rank_add_or_delete.inc');

function create_class($classname, $with_first_round = true) {
  global $db;
  if (schema_version() >= 2) {
    $sortorder = read_single_value('SELECT 1 + MAX(sortorder) FROM Classes');
    if (!$sortorder) {
      $sortorder = read_single_value('SELECT 1 + COUNT(*) FROM Classes');
    }
  }
  $stmt = $db->prepare('INSERT INTO Classes(class) VALUES(:classname)');
  $stmt->execute(array(':classname' => $classname));
  $classid = read_single_value('SELECT classid FROM Classes WHERE class = :classname',
                               array(':classname' => $classname));
  if (schema_version() >= 2) {
    record_event(EVENT_CLASS_ADDED,
                 array('classid' => $classid,
                       'other' => $classname));
    $stmt = $db->prepare('UPDATE Classes'
                         .' SET sortorder = :sortorder'
                         .' WHERE classid = :classid');
    $stmt->execute(array(':classid' => $classid,
                         ':sortorder' => $sortorder));
  }

  // Creating a class usually requires creating a corresponding 1st racing round
  if ($with_first_round) {
    // TODO make_new_round from roster.inc
    $stmt = $db->prepare('INSERT INTO Rounds(round, classid)'
                         .' SELECT 1, classid'
                         .' FROM Classes'
                         .' WHERE classid = :classid'
                         .' AND NOT EXISTS(SELECT 1 FROM Rounds'
                         .'  WHERE Rounds.classid = Classes.classid'
                         .'    AND Rounds.round = 1)');
    $stmt->execute(array(':classid' => $classid));

    $roundid = read_single_value('SELECT roundid FROM Rounds'
                                 .' WHERE round = 1'
                                 .'  AND classid = :classid',
                                 array(':classid' => $classid));
    record_event(EVENT_ROUND_ADDED, array('roundid' => $roundid,
                                          'classid' => $classid,
                                          'other' => 'round 1'));
  }

  return $classid;
}

function find_or_create_class($classname, $with_first_round = true) {
  global $db;
  $classname = substr(trim($classname), 0, CLASS_COLUMN_WIDTH);
  $classid = read_single_value('SELECT classid FROM Classes WHERE class = :classname',
                               array(':classname' => $classname));
  if ($classid === false) {
    $classid = create_class($classname, $with_first_round);
  }

  return $classid;
}

// Starting from $seed, come up with a string that's not the name of an existing class.
function choose_unused_class_name($seed) {
  global $db;
  if (read_single_value('SELECT COUNT(*) FROM Classes WHERE class = :classname',
                        array(':classname' => $seed)) == 0) {
    // Use the original string, if possible
    return $seed;
  }
  for ($i = 1; true; ++$i) {
    $classname = $seed.'-'.$i;
    if (read_single_value('SELECT COUNT(*) FROM Classes WHERE class = :classname',
                          array(':classname' => $classname)) == 0) {
      return $classname;
    }
  }
}

function delete_class($classid) {
  global $db;

  $classname = read_single_value('SELECT class FROM Classes WHERE classid = :classid',
                                 array(':classid' => $classid), '(No such class)');

  $stmt = $db->prepare('SELECT rankid FROM Ranks WHERE classid = :classid');
  $stmt->execute(array(':classid' => $classid));
  foreach ($stmt->fetchAll(PDO::FETCH_COLUMN, 0) as $rankid) {
    delete_rank($rankid);
  }

  $stmt = $db->prepare('DELETE FROM RaceChart'
                       .' WHERE roundid IN'
                       .'  (SELECT roundid FROM Rounds'
                       .'   WHERE classid = :classid)');
  $stmt->execute(array(':classid' => $classid));
  if ($stmt->rowCount() > 0) {
    record_event(EVENT_SCHEDULE_DELETED, array('roundid' => -1,
                                               'other' => "Deleting class $classname"));
  }

  // There might be Roster entries for an aggregate class
  // For an organic class, the Roster table shouldn't have any entries for the
  // class to be deleted, because the caller should have confirmed there are no racers in the class.
  $stmt = $db->prepare('DELETE FROM Roster WHERE roundid IN'
                       .'  (SELECT roundid FROM Rounds'
                       .'   WHERE classid = :classid)');
  $stmt->execute(array(':classid' => $classid));
  
  if (read_single_value('SELECT COUNT(*) FROM Rounds WHERE classid = :classid',
                        array(':classid' => $classid)) > 0) {
    
    record_event(EVENT_ROUND_DELETED, array('classid' => $classid,
                                            'other' => $classname.', all rounds'));
    $stmt = $db->prepare('DELETE FROM Rounds  WHERE classid = :classid');
    $stmt->execute(array(':classid' => $classid));
  }

  $stmt = $db->prepare('DELETE FROM Awards WHERE classid = :classid');
  $stmt->execute(array(':classid' => $classid));

  $stmt = $db->prepare('DELETE FROM Playlist WHERE classid = :classid');
  $stmt->execute(array(':classid' => $classid));

  $stmt = $db->prepare('DELETE FROM ConstituentClasses WHERE dependent = :classid');
  $stmt->execute(array(':classid' => $classid));

  $stmt = $db->prepare('DELETE FROM Classes WHERE classid = :classid');
  $stmt->execute(array(':classid' => $classid));
  if ($stmt->rowCount() > 0) {
    record_event(EVENT_CLASS_DELETED, array('classid' => $classid, 'other' => $classname));
  }
}

?>
