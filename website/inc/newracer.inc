<?php
require_once('inc/schema_version.inc');
require_once('inc/sql-script.inc');
require_once(sql_file_path('column-widths'));
require_once('inc/class_add_or_delete.inc');
require_once('inc/rank_add_or_delete.inc');
require_once('inc/partitions.inc');
require_once('inc/events.inc');

define('UNSPECIFIED_CLASS', '* Not specified');
define('UNSPECIFIED_RANK', '* Not specified');

// $values are: { firstname, lastname, carnumber, carname, partitionid, partition, exclude }
//   partition is ignored if partitionid is provided.
//   If neither partition nor partitionid is provided, default partition name is assumed
// Returns the racerid for the new racer.

function insert_new_racer($values) {
  global $db;

  if (!isset($values['classid']) && isset($values['rankid'])) {
    $values['classid'] = read_single_value('SELECT classid FROM Ranks WHERE rankid = :rankid',
                                           array(':rankid' => $values['rankid']));
  }

  if (!isset($values['partitionid']) && !isset($values['partition']) && isset($values['classid'])) {
    // TODO This is an interim arrangement only
    $values['partitionid'] = read_single_value('SELECT partitionid FROM Partitions'
                                              .' WHERE name = (SELECT class FROM Classes'
                                              .'                WHERE classid = :classid)',
                                              array(':classid' => $values['classid']));
  }

  if (!isset($values['partitionid'])) {
    $values['partitionid'] = find_or_create_partition(
      (isset($values['partition']) && !empty($values['partition']))
          ? $values['partition'] : DEFAULT_PARTITION_NAME);
  }
  
  if (!isset($values['carnumber']) || $values['carnumber'] == '') {
    $max_carnumber = read_single_value('SELECT MAX(carnumber) FROM RegistrationInfo'
                                       .' WHERE partitionid = :partitionid',
                                       array(':partitionid' => $values['partitionid']));
    if (!$max_carnumber) {
      $max_carnumber = ($values['partitionid'] - 1) * 100;
    }
    $values['carnumber'] = $max_carnumber + 1;
  }

  if (read_single_value('SELECT COUNT(*) FROM RegistrationInfo'
                        .' WHERE carnumber = :carnumber',
                        array(':carnumber' => $values['carnumber'])) > 0) {
    json_out('warnings', array("Duplicate car number $values[carnumber]"));
  }

  if (!isset($values['classid'])) {
    list($classid, $rankid) = read_single_row('SELECT classid, rankid FROM Ranks'
                                              .' WHERE rankid = (SELECT rankid FROM Partitions'
                                              .'                 WHERE partitionid = :partitionid)',
                                              array(':partitionid' => $values['partitionid']));
    $values['classid'] = $classid;
    $values['rankid'] = $rankid;
  }


  $stmt = $db->prepare('INSERT INTO RegistrationInfo (carnumber, lastname, firstname, carname,'
                       .'                             note, partitionid, rankid, classid, exclude)'
                       .' VALUES(:carnumber, :lastname, :firstname, :carname,'
                       .'        :note, :partitionid, :rankid, :classid, :exclude)');
  $stmt->execute(array(':carnumber' => trim($values['carnumber']),
                       ':carname' => trim($values['carname']),
                       ':note' => trim($values['note_from']),
                       ':firstname' => trim($values['firstname']),
                       ':lastname' => trim($values['lastname']),
                       ':partitionid' => $values['partitionid'],
                       ':rankid' => $values['rankid'],
                       ':classid' => $values['classid'],
                       ':exclude' => (isset($values['exclude']) && $values['exclude']) ? 1 : 0));

  // The new racer won't be recognized without a Roster record to go with it.
  fill_in_missing_roster_entries();

  // Return racerid
  $stmt = $db->prepare('SELECT racerid FROM RegistrationInfo'
                       .' WHERE firstname = :firstname and lastname = :lastname'
                       .'  AND partitionid = :partitionid');
  $stmt->execute(array(':firstname' => trim($values['firstname']),
                       ':lastname' => trim($values['lastname']),
                       ':partitionid' => $values['partitionid']));
  $rs = $stmt->fetch(PDO::FETCH_NUM);
  return $rs[0];
}

// This just enrolls everyone into Round 1 for their Class.
function fill_in_missing_roster_entries() {
  global $db;
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
