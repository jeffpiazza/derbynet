<?php

// $_POST['awardid']
// $_POST['votes']

require_once('inc/voterid.inc');

$ballot_code = session_id();
list($voterid, $ballotid) = retrieve_voterid($ballot_code);

$ok = true;
if (!$voterid) {
  json_failure('invalid', "Invalid ballot code");
  $ok = false;
}
if ($ok && read_raceinfo('balloting', 'closed') == 'closed') {
  json_failure('closed', "Voting is not open at this time");
  $ok = false;
}

$awardid = $_POST['awardid'];
if ($ok) {
  $award = read_single_row('SELECT classid, rankid FROM Awards'
                           .' WHERE awardid = :awardid',
                           array(':awardid' => $awardid),
                           PDO::FETCH_ASSOC);
  if (!$award) {
    json_failure('no-award', "No such award");
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
  json_failure('too-many', "Too many choices offered");
  $ok = false;
}

if ($ok) {
    $stmt = $db->prepare('DELETE FROM Votes'
                         .' WHERE voterid = :voterid'
                         .'  AND awardid = :awardid');
    $stmt->execute(array(':voterid' => $voterid,
                         ':awardid' => $awardid));

    $vote_summary = array();
    $stmt = $db->prepare('INSERT INTO Votes(awardid, racerid, voterid)'
                         .' VALUES(:awardid, :racerid, :voterid)');
    foreach ($votes as $racerid) {
      $stmt->execute(array(':awardid' => $awardid,
                           ':racerid' => $racerid,
                           ':voterid' => $voterid));
      $vote_summary[] = "$voterid $awardid $racerid";
    }
    json_out('votes', $vote_summary);
}

if ($ok) {
  json_success();
}

?>