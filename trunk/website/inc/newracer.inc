<?php


function insert_new_racer($firstname, $lastname, $carno, $rank_and_den, $exclude = false) {
    // $rank_and_den is <rankid>,<classid>
    global $db;

    $stmt = $db->prepare('INSERT INTO RegistrationInfo (CarNumber, LastName, FirstName, RankID, ClassID, Exclude)'
                         .' VALUES(:carno, :lastname, :firstname, '.$rank_and_den.', :exclude)');
    $stmt->execute(array(':carno' => $carno,
                         ':firstname' => $firstname,
                         ':lastname' => $lastname,
                         ':exclude' => $exclude ? 1 : 0));

    // The new racer won't be recognized without a Roster record to go with it.
    // Rounds has ChartType and Phase fields whose meaning isn't obvious.  This just enrolls
    // everyone into Round 1 for their Class.
    $db->exec('INSERT INTO Roster(RoundID, ClassID, RacerID)'
              .' SELECT RoundID, RegistrationInfo.ClassID, RacerID'
              .' FROM Rounds'
              .' INNER JOIN RegistrationInfo'
              .' ON Rounds.ClassID = RegistrationInfo.ClassID'
              .' WHERE Round = 1'
              .' AND NOT EXISTS(SELECT 1 FROM Roster'
              .' WHERE Roster.RoundID = Rounds.RoundID'
              .' AND Roster.ClassID = RegistrationInfo.ClassID'
              .' AND Roster.RacerID = RegistrationInfo.RacerID)');
}

?>