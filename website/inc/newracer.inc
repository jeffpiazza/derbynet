<?php
require_once('inc/schema_version.inc');

// $values are: { firstname, lastname, carnumber, carname, rankid, classid, exclude }
function insert_new_racer($values) {
  global $db;

  if (!isset($values['classid'])) {
    $values['classid'] = read_single_value('SELECT classid FROM Ranks WHERE rankid = :rankid',
                                           array(':rankid' => $values['rankid']));
  }

  $stmt = $db->prepare('INSERT INTO RegistrationInfo (carnumber, lastname, firstname, carname,'
                       .'                             rankid, classid, exclude)'
                       .' VALUES(:carno, :lastname, :firstname, :carname, :rankid, :classid, :exclude)');
  $stmt->execute(array(':carno' => trim($values['carnumber']),
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

// Manufactures a classid and rankid when there are none.  This arises only if
// creating racers from an empty roster on the check-in page.
function force_populate_a_rankid() {
  global $db;
  if (read_single_value('SELECT COUNT(*) FROM Ranks') == 0) {
    if (read_single_value('SELECT COUNT(*) FROM Classes') == 0) {
      $stmt = $db->prepare("INSERT INTO Classes(class) VALUES(:classname)");
      $stmt->execute(array(':classname' => '* Not specified'));
      if (schema_version() >= 2) {
        $stmt = $db->prepare('UPDATE Classes SET sortorder = 1');
        $stmt->execute(array());
      }
    }
    // If there are classes but not ranks, we'll pick a class at random, but most likely,
    // the only class in the database is the one we just created.
    $classid = read_single_value('SELECT classid FROM Classes');
    $stmt = $db->prepare("INSERT INTO Ranks(classid, rank) VALUES(:classid, :rankname)");
    $stmt->execute(array(':classid' => $classid,
                         ':rankname' => '* Not specified'));
    if (schema_version() >= 2) {
      $stmt = $db->prepare('UPDATE Ranks SET sortorder = 1 WHERE classid = :classid');
      $stmt->execute(array(':classid' => $classid));
    }
  }
  // Pick a rankid at random, if none was specified
  return read_single_value('SELECT rankid FROM Ranks');
}

?>