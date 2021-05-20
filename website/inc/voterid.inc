<?php

require_once('inc/schema_version.inc');

// Only support for one ballot for now, with this distinguished ballotid.
define('BALLOTID', 1);

function validate_ballot_code($s) {
  // Allow any ballot code for now.
  return true;
}

// Returns the voterid and ballotid for this voter
function retrieve_voterid($ballot_code) {
  global $db;
  $voter = read_single_row('SELECT voterid, ballotid FROM Voters WHERE identifier = :ballotcode',
                           array(':ballotcode' => $ballot_code));
  if ($voter === false) {
    $stmt = $db->prepare('INSERT INTO Voters(ballotid, identifier, when_cast, ip_addr)'
                         .' VALUES(:ballotid, :ballotcode, '.dbnow_expr().', :ipaddr)');
    $stmt->execute(array(':ballotid' => BALLOTID,
                         ':ballotcode' => $ballot_code,
                         ':ipaddr' => $_SERVER['REMOTE_ADDR']));
    $voter = read_single_row('SELECT voterid, ballotid FROM Voters WHERE identifier = :ballotcode',
                             array(':ballotcode' => $ballot_code));
  }
  return array($voter[0], $voter[1]);
}

// Returns array of { awardid awardname candidates },
//  with each candidate entry being { score firstname lastname carnumber }
function vote_tabulations() {
  global $db;

  $stmt = $db->prepare('SELECT Votes.awardid, awardname, Awards.racerid AS recipient,'
                       .' Votes.racerid, COUNT(*) AS score,'
                       .' firstname, lastname, carnumber'
                       .' FROM '.inner_join('Votes', 'Awards',
                                            'Votes.awardid = Awards.awardid',
                                            'RegistrationInfo',
                                            'Votes.racerid = RegistrationInfo.racerid')
                       .' GROUP BY Votes.awardid, Votes.racerid'
                       .' ORDER BY Awards.sort, Awards.awardid, COUNT(*) DESC');
  $stmt->execute();

  $tab = array();
  foreach ($stmt as $row) {
    if (count($tab) == 0 || $row['awardid'] != $tab[count($tab) - 1]['awardid']) {
      $tab[] = array('awardid' => $row['awardid'],
                     'awardname' => $row['awardname'],
                     'recipient' => $row['recipient'],  // Racer id of current recipient, 0=none
                     'candidates' => array());
    }
    $tab[count($tab) - 1]['candidates'][] =
        array('score' => $row['score'],
              'carnumber' => $row['carnumber'],
              'firstname' => $row['firstname'],
              'lastname' => $row['lastname']);
  }
  return $tab;
}

function set_ballot_depth($awardid, $ballot_depth) {
  global $db;
  if (schema_version() < BALLOTING_SCHEMA) {
    return;
  }

  $ballotid = BALLOTID;

  $stmt = $db->prepare('DELETE FROM BallotAwards'
                       .' WHERE awardid = :awardid'
                       .'   AND ballotid = :ballotid');
  $stmt->execute(array(':awardid' => $awardid,
                       ':ballotid' => $ballotid));

  if ($ballot_depth > 0) {
    $stmt = $db->prepare('INSERT INTO BallotAwards(ballotid, awardid, depth)'
                         .' VALUES(:ballotid, :awardid, :ballot_depth)');
    $stmt->execute(array(':ballotid' => $ballotid,
                         ':awardid' => $awardid,
                         ':ballot_depth' => $ballot_depth));
  } else {
    // Remove previously-cast votes when removing from ballot
    $stmt = $db->prepare('DELETE FROM Votes WHERE awardid = :awardid');
    $stmt->execute(array(':awardid' => $awardid));
  }
}
?>