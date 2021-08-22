<?php
require_once('inc/schema_version.inc');
require_once('inc/sql-script.inc');
require_once(sql_file_path('column-widths'));
require_once('inc/find_or_create_class.inc');
require_once('inc/divisions.inc');
require_once('inc/events.inc');

define('UNSPECIFIED_CLASS', '* Not specified');
define('UNSPECIFIED_RANK', '* Not specified');

// $values are: { firstname, lastname, carnumber, carname, divisionid, division, exclude }
//   division is ignored if divisionid is provided.
//   If neither division nor divisionid is provided, 'Default' is assumed

function insert_new_racer($values) {
  global $db;

  if (!isset($values['classid']) && isset($values['rankid'])) {
    $values['classid'] = read_single_value('SELECT classid FROM Ranks WHERE rankid = :rankid',
                                           array(':rankid' => $values['rankid']));
  }

  if (!isset($values['divisionid']) && !isset($values['division']) && isset($values['classid'])) {
    // TODO This is an interim arrangement only
    $values['divisionid'] = read_single_value('SELECT divisionid FROM Divisions'
                                              .' WHERE name = (SELECT class FROM Classes'
                                              .'                WHERE classid = :classid)',
                                              array(':classid' => $values['classid']));
  }

  if (!isset($values['divisionid'])) {
    $values['divisionid'] = find_or_create_division(
      isset($values['division']) ? $values['division'] : 'Default');
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

  if (!isset($values['classid'])) {
    list($classid, $rankid) = read_single_row('SELECT classid, rankid FROM Ranks'
                                              .' WHERE rankid = (SELECT rankid FROM Divisions'
                                              .'                 WHERE divisionid = :divisionid)',
                                              array(':divisionid' => $values['divisionid']));
    $values['classid'] = $classid;
    $values['rankid'] = $rankid;
  }
  json_out('debug', $values);

  $stmt = $db->prepare('INSERT INTO RegistrationInfo (carnumber, lastname, firstname, carname,'
                       .'                             divisionid, rankid, classid, exclude)'
                       .' VALUES(:carnumber, :lastname, :firstname, :carname,'
                       .'        :divisionid, :rankid, :classid, :exclude)');
  $stmt->execute(array(':carnumber' => trim($values['carnumber']),
                       ':carname' => trim($values['carname']),
                       ':firstname' => trim($values['firstname']),
                       ':lastname' => trim($values['lastname']),
                       ':divisionid' => $values['divisionid'],
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