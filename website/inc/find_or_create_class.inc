<?php

require_once('inc/schema_version.inc');
require_once('inc/sql-script.inc');
require_once(sql_file_path('column-widths'));
require_once('inc/events.inc');

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

function find_or_create_rank($rankname, $classid) {
  global $db;
  $rankname = substr(trim($rankname), 0, RANK_COLUMN_WIDTH);
  $rankid = read_single_value('SELECT rankid FROM Ranks'
                              .' WHERE rank = :rankname AND classid = :classid',
                              array(':rankname' => $rankname,
                                    ':classid' => $classid));
  if ($rankid === false) {
    $stmt = $db->prepare('INSERT INTO Ranks(rank, classid) VALUES(:rankname, :classid)');
    $stmt->execute(array(':rankname' => $rankname,
                         ':classid' => $classid));
    $rankid = read_single_value('SELECT rankid FROM Ranks'
                                .' WHERE rank = :rankname AND classid = :classid',
                                array(':rankname' => $rankname,
                                      ':classid' => $classid));
    if (schema_version() >= 2) {
      record_event(EVENT_RANK_ADDED, array('classid' => $classid,
                                           'rankid' => $rankid,
                                           'other' => $rankname));
      $sortorder = read_single_value('SELECT 1 + MAX(sortorder) FROM Ranks WHERE classid = :classid',
                                     array(':classid' => $classid));
      if ($sortorder === false) {
        $sortorder = 1;
      }
      $stmt = $db->prepare('UPDATE Ranks'
                           .' SET sortorder = :sortorder'
                           .' WHERE rankid = :rankid');
      $stmt->execute(array(':sortorder' => $sortorder,
                           ':rankid' => $rankid));
    }
  }

  return $rankid;
}
?>