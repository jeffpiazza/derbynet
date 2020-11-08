<?php

// $_POST['awardid']
// $_POST['votes']

require_once('inc/voterid.inc');

start_response();

$ballot_code = session_id();
list($voterid, $ballotid) = retrieve_voterid($ballot_code);

$ok = true;
if (!$voterid) {
  echo "<failure code='invalid'>Invalid ballot code</failure>\n";
  $ok = false;
}
if ($ok && read_raceinfo('balloting', 'closed') == 'closed') {
  echo "<failure code='closed'>Voting is not open at this time</failure>\n";
  $ok = false;
}

$awardid = $_POST['awardid'];
if ($ok) {
  $award = read_single_row('SELECT classid, rankid FROM Awards'
                           .' WHERE awardid = :awardid',
                           array(':awardid' => $awardid),
                           PDO::FETCH_ASSOC);
  if (!$award) {
    echo "<failure code='no-award'>No such award</failure>\n";
    $ok = false;
  }
}

$votes = json_decode($_POST['votes'], true);

if ($ok && count($votes) >
    read_single_value('SELECT depth FROM BallotAwards'
                      .' WHERE ballotid = :ballotid'
                      .'   AND awardid = :awardid',
                      array(':ballotid' => BALLOTID,
                            ':awardid' => $awardid))) {
    echo "<failure code='too-many'>Too many choices offered</failure>\n";
    $ok = false;
}

if ($ok) {
    $stmt = $db->prepare('DELETE FROM Votes'
                         .' WHERE voterid = :voterid'
                         .'  AND awardid = :awardid');
    $stmt->execute(array(':voterid' => $voterid,
                         ':awardid' => $awardid));

    $stmt = $db->prepare('INSERT INTO Votes(awardid, racerid, voterid)'
                         .' VALUES(:awardid, :racerid, :voterid)');
    foreach ($votes as $racerid) {
      $stmt->execute(array(':awardid' => $awardid,
                           ':racerid' => $racerid,
                           ':voterid' => $voterid));
      echo "<vote>$voterid $awardid $racerid</vote>\n";
    }
}

if ($ok) {
  echo "\n<success/>\n";
}

end_response();

?>