<?php

// $_GET['password']
// Optional $_GET['ballot_code']

require_once('inc/voterid.inc');
require_once('inc/schema_version.inc');

$pwd = isset($_GET['password']) ? $_GET['password'] : '';

if (schema_version() < 6) {
  json_failure('old-db', "No balloting tables.");
} else if ($pwd != read_raceinfo('ballot_password', '') && !isset($_SESSION['ok-to-vote'])) {
  json_failure('password', "Wrong password for ballot");
} else {
  $_SESSION['ok-to-vote'] = 1;
  $ballot_code = session_id();
  if (isset($_GET['ballot_code'])) {
    $ballot_code = $_GET['ballot_code'];
  }

  list($voterid, $ballotid) = retrieve_voterid($ballot_code);

  if (!$voterid) {
    json_failure('invalid', "Invalid ballot code");
  } else {
    $ballot = array();
    if (read_raceinfo('balloting', 'closed') != 'closed') {
      $stmt = $db->prepare('SELECT awardid, depth FROM BallotAwards WHERE ballotid = :ballotid');
      $stmt->execute(array(':ballotid' => $ballotid));
      foreach ($stmt as $row) {
        $ballot[$row['awardid']] = array('max_votes' => $row['depth'], 'votes' => []);
      }

      $stmt = $db->prepare('SELECT awardid, racerid FROM Votes'
                           .' WHERE voterid = :voterid'
                           .' ORDER BY awardid');
      $stmt->execute(array(':voterid' => $voterid));

      foreach ($stmt as $row) {
        // TODO What if there's a vote recorded for an award that's not available
        // for balloting?
        $ballot[$row['awardid']]['votes'][] = intval($row['racerid']);
      }
    }
    if (empty($ballot)) {
      $ballot = new stdClass();
    }
    json_out('ballot', $ballot);
  }
}

?>
