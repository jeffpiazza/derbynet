<?php


function insert_new_racer_rankid($firstname, $lastname, $carno, $rankid, $exclude = false) {
  global $db;

  $classid = read_single_value('SELECT classid FROM Ranks WHERE rankid = :rankid',
                               array(':rankid' => $rankid));

  insert_new_racer_rankid_and_classid($firstname, $lastname, $carno, $rankid, $classid, $exclude);
}

function insert_new_racer_rankid_and_classid($firstname, $lastname, $carno, $rankid, $classid, $exclude = false) {
    // $rank_and_den is <rankid>,<classid>
    global $db;

    $stmt = $db->prepare('INSERT INTO RegistrationInfo (carnumber, lastname, firstname, rankid, classid, exclude)'
                         .' VALUES(:carno, :lastname, :firstname, :rankid, :classid, :exclude)');
    $stmt->execute(array(':carno' => $carno,
                         ':firstname' => $firstname,
                         ':lastname' => $lastname,
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