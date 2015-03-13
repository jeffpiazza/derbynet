<?php


function insert_new_racer_rankid($firstname, $lastname, $carno, $carname, $rankid, $exclude = false) {
  global $db;

  $classid = read_single_value('SELECT classid FROM Ranks WHERE rankid = :rankid',
                               array(':rankid' => $rankid));

  insert_new_racer_rankid_and_classid($firstname, $lastname, $carno, $carname, $rankid, $classid, $exclude);
}

function insert_new_racer_rankid_and_classid($firstname, $lastname, $carno, $carname,
                                             $rankid, $classid, $exclude = false) {
    global $db;

    $stmt = $db->prepare('INSERT INTO RegistrationInfo (carnumber, lastname, firstname, carname,'
                         .'                             rankid, classid, exclude)'
                         .' VALUES(:carno, :lastname, :firstname, :carname, :rankid, :classid, :exclude)');
    $stmt->execute(array(':carno' => trim($carno),
                         ':carname' => trim($carname),
                         ':firstname' => trim($firstname),
                         ':lastname' => trim($lastname),
                         ':rankid' => $rankid,
                         ':classid' => $classid,
                         ':exclude' => $exclude ? 1 : 0));

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
              .' AND Roster.classid = RegistrationInfo.classid'
              .' AND Roster.racerid = RegistrationInfo.racerid)');
}

?>