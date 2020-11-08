<?php

// $_GET['password']
// Optional $_GET['ballot_code']

require_once('inc/voterid.inc');
require_once('inc/schema_version.inc');

echo "<get-ballot>\n";

$pwd = isset($_GET['password']) ? $_GET['password'] : '';

if (schema_version() < 6) {
  echo "<failure code='old-db'>No balloting tables.</failure>\n";
} else if ($pwd != read_raceinfo('ballot_password', '') && !isset($_SESSION['ok-to-vote'])) {
  echo "<failure code='password'>Wrong password for ballot</failure>\n";
} else {
  $_SESSION['ok-to-vote'] = 1;
  $ballot_code = session_id();
  if (isset($_GET['ballot_code'])) {
    $ballot_code = $_GET['ballot_code'];
  }

  list($voterid, $ballotid) = retrieve_voterid($ballot_code);

  if (!$voterid) {
    echo "<failure code='invalid'>Invalid ballot code</failure>\n";
  } else if (read_raceinfo('balloting', 'closed') == 'closed') {
    echo "<ballot>{}</ballot>\n";
  } else {
    $ballot = array();
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
    echo "<ballot>".(empty($ballot) ? "{}" : json_encode($ballot))."</ballot>\n";
  }
}

echo "</get-ballot>\n";

?>
