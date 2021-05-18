<?php
require_once('inc/schema_version.inc');
require_once('inc/sql-script.inc');
require_once(sql_file_path('column-widths'));
require_once('inc/events.inc');

define('UNSPECIFIED_CLASS', '* Not specified');
define('UNSPECIFIED_RANK', '* Not specified');

// $values are: { firstname, lastname, carnumber, carname, rankid, classid, exclude }
// Must have at least one of classid or rankid.

function insert_new_racer($values) {
  global $db;

  if (!isset($values['classid'])) {
    $values['classid'] = read_single_value('SELECT classid FROM Ranks WHERE rankid = :rankid',
                                           array(':rankid' => $values['rankid']));
  }

  if (!isset($values['carnumber']) || $values['carnumber'] == '') {
    $max_carnumber = read_single_value('SELECT MAX(carnumber) FROM RegistrationInfo'
                                       .' WHERE rankid = :rankid',
                                       array(':rankid' => $values['rankid']));
    if (!$max_carnumber) {
      $max_carnumber = $values['rankid'] * 100;
    }
    $values['carnumber'] = $max_carnumber + 1;
  }

  if (read_single_value('SELECT COUNT(*) FROM RegistrationInfo'
                        .' WHERE carnumber = :carnumber',
                        array(':carnumber' => $values['carnumber'])) > 0) {
    json_out('warnings', array("Duplicate car number $values[carnumber]"));
  }

  $stmt = $db->prepare('INSERT INTO RegistrationInfo (carnumber, lastname, firstname, carname,'
                       .'                             rankid, classid, exclude)'
                       .' VALUES(:carnumber, :lastname, :firstname, :carname, :rankid, :classid, :exclude)');
  $stmt->execute(array(':carnumber' => trim($values['carnumber']),
                       ':carname' => trim($values['carname']),
                       ':firstname' => trim($values['firstname']),
                       ':lastname' => trim($values['lastname']),
                       ':rankid' => $values['rankid'],
                       ':classid' => $values['classid'],
                       ':exclude' => (isset($values['exclude']) && $values['exclude']) ? 1 : 0));

  // The new racer won't be recognized without a Roster record to go with it.
  // Rounds has ChartType and Phase fields whose meaning isn't obvious.  This just enrolls
  // everyone into Round 1 for their Class.
  $db->exec('INSERT INTO Roster(roundid, classid, racerid)'
            .' SELECT roundid, RegistrationInfo.classid, racerid'
            .' FROM Rounds'
            .' INNER JOIN RegistrationInfo'
            .' ON Rounds.classid = RegistrationInfo.classid'
            .' WHERE round = 1'
            .' AND NOT EXISTS(SELECT 1 FROM Roster'
            .' WHERE Roster.roundid = Rounds.roundid'
            // The classid field on the Roster is a mis-design inherited from
            // GPRM.  The field is completely redundant, which just means,
            // sooner or later, it'll get the wrong value.
            //
            // A racer is enrolled in a round if there's a Roster entry connecting
            // the racerid to the roundid; the classid shouldn't matter.
            //
            // .' AND Roster.classid = RegistrationInfo.classid'
            .' AND Roster.racerid = RegistrationInfo.racerid)');
  
}

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
    record_event_create_class($classid, $classname);
    $stmt = $db->prepare('UPDATE Classes'
                         .' SET sortorder = :sortorder'
                         .' WHERE classid = :classid');
    $stmt->execute(array(':classid' => $classid,
                         ':sortorder' => $sortorder));
  }

  // Creating a class usually requires creating a corresponding 1st racing round
  if ($with_first_round) {
    $stmt = $db->prepare('INSERT INTO Rounds(round, classid)'
                         .' SELECT 1, classid'
                         .' FROM Classes'
                         .' WHERE classid = :classid'
                         .' AND NOT EXISTS(SELECT 1 FROM Rounds'
                         .'  WHERE Rounds.classid = Classes.classid'
                         .'    AND Rounds.round = 1)');
    $stmt->execute(array(':classid' => $classid));
    record_event_create_round(-1, $classid, 1);
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
      record_event_create_rank($classid, $rankid, $rankname);
      $nranks = read_single_value('SELECT COUNT(*) FROM Ranks WHERE classid = :classid',
                                  array(':classid' => $classid));
      $stmt = $db->prepare('UPDATE Ranks'
                           .' SET sortorder = :nranks'
                           .' WHERE rankid = :rankid');
      $stmt->execute(array(':nranks' => $nranks,
                           ':rankid' => $rankid));
    }
  }

  return $rankid;
}

// Manufactures a classid and rankid when there are none.  This arises only if
// creating racers from an empty roster on the check-in page.
function force_populate_a_rankid() {
  global $db;
  if (read_single_value('SELECT COUNT(*) FROM Classes') == 0) {
    $classid = find_or_create_class(UNSPECIFIED_CLASS);
  } else {
    // If there are classes but not ranks, we'll pick a class at random, but most likely,
    // the only class in the database is the one we just created.
    $classid = read_single_value('SELECT classid FROM Classes');
  }

  if (read_single_value('SELECT COUNT(*) FROM Ranks WHERE classid = :classid',
                        array(':classid' => $classid)) == 0) {
    $rankid = find_or_create_rank(UNSPECIFIED_RANK, $classid);
  } else {
    $rankid = read_single_value('SELECT rankid FROM Ranks WHERE classid = :classid',
                                array(':classid' => $classid));
  }

  return $rankid;
}

?>