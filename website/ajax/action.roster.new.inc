<?php
// $_POST roundid, roundid_<nnn>, top, bucketed

// Populates a new round by selecting top finishers from a previous round or rounds.

// If $_POST['roundid'] is not empty, it identifies a completed round, and
// implies that we're populating a new follow-on round for the same class, based
// on the top finishers from the identified round.
//
// If roundid is empty, then we're generating a "Grand Finals" round, taking
// top finishers across multiple previous rounds.  Those previous rounds are
// identified by $_POST keys of the form roundid_<nnn>.
//
// "top" identifies the how many racers to select for the new round, but its interpretation also
// depends on "bucketed".
//
// For a Grand Finals round, if "bucketed" is true, then "top" tells how many
// racers to choose from each of the prior rounds feeding in to the new round.
//
// For a non-Grand Finals round, if "bucketed" is true, then "top" tells how
// many racers to choose from each subgroup.
//
// TODO: If racing isn't complete, selection of top finishers is likely to be wrong.

start_response();

require_once('inc/standings.inc');

$bucket_limit = $_POST['top'];
$bucketed = isset($_POST['bucketed']) && $_POST['bucketed'];

if (isset($_POST['roundid']) && strlen($_POST['roundid']) > 0) {
  $grand_finals = false;
  $roundid = $_POST['roundid'];
  list($classid, $old_round) = read_single_row('SELECT classid, round FROM Rounds'
                                               .' WHERE roundid = :roundid',
                                               array(':roundid' => $roundid));
  // TODO: Handle not-found case
  $new_round = 1 + $old_round;
} else {
  $grand_finals = true;
  $all_roundids = '0';
  foreach ($_POST as $key => $value) {
    if (substr($key, 0, strlen('roundid_')) == 'roundid_') {
      $all_roundids .= ', '.substr($key, strlen('roundid_'));
    }
  }
  // TODO: Handle 0 or 1 roundid's provided

  // A Grand Final round has the same round number as the highest completed
  // round it draws from.  In other words, if you're picking top finishers from
  // several Round 1 rounds, you're constructing a Round 1 Grand Final round.
  $old_round = read_single_value('SELECT MAX(round)'
                                 .' FROM RaceChart'
                                 .' INNER JOIN Rounds'
                                 .' ON RaceChart.roundid = Rounds.roundid'
                                 .' WHERE (finishtime IS NOT NULL OR finishplace IS NOT NULL)'
                                 .' AND Rounds.roundid IN ('.$all_roundids.')',
                                 array());
  $new_round = $old_round;

  // Get or insert a 'Grand Finals' class.
  $classid = read_single_value("SELECT classid FROM Classes WHERE class = 'Grand Finals'",
                               array(), 0);
  if (!$classid) {
    $db->exec("INSERT INTO Classes(class) VALUES('Grand Finals')");
    $classid = read_single_value("SELECT classid FROM Classes WHERE class = 'Grand Finals'",
                                 array(), 0);
    if (expected_schema_version() >= 2) {
      // Apparently mysql can't support an embedded select from the same table
      // that's being updated, so this has to be done in two queries.
      $max = read_single_value('SELECT MAX(sortorder) FROM Classes');
      $stmt = $db->prepare("UPDATE Classes SET sortorder = 1 + :max"
                           ." WHERE classid = :classid");
      $stmt->execute(array(':classid' => $classid,
                           ':max' => $max));
    }
  }
}

// With $new_round and $classid settled, make the new round (unless it
// already exists)

// TODO: This won't work in general if there's just one "Grand Finals"
// class and potentially multiple desired rounds at a particular round
// level.  Probably need to keep incrementing new round numbers until
// we get one that hasn't already been used.
$new_roundid = read_single_value('SELECT roundid FROM Rounds'
                                 .' WHERE round = :round'
                                 .'  AND classid = :classid',
                                 array(':round' => $new_round,
                                       ':classid' => $classid),
                                 0);
if ($new_roundid) {
  // If found a Rounds entry, then we're repeating making a roster.
  // Delete the old entries.
  $stmt = $db->prepare('DELETE FROM Roster WHERE roundid = :roundid');
  $stmt->execute(array(':roundid' => $new_roundid));
} else {
  // The usual case: make a whole new round
  // phase/charttype are for GPRM compatibility only; I really don't know what they mean
  $stmt = $db->prepare('INSERT INTO Rounds(round, classid, phase, charttype) VALUES(:round, :classid, 0, 0)');
  $stmt->execute(array(':round' => $new_round,
                       ':classid' => $classid));
  $stmt->closeCursor();
  $new_roundid = read_single_value('SELECT roundid FROM Rounds'
                                   .' WHERE round = :round'
                                   .'  AND classid = :classid',
                                   array(':round' => $new_round,
                                         ':classid' => $classid));
}

$buckets = array();  // bucketno => array of { racerid , score }

// If top n across the whole unit or den, bucketno is a constant (1)
// If bucketed, bucketno is rankid (for a den) or classid (for grand finals).
$bucketno_expr = '1';

if ($grand_finals) {
  $and_roundid_ok = ' AND RaceChart.roundid IN ('.$all_roundids.')';
  $keyfield = 'Rounds.round';
  $key = $old_round;
  if ($bucketed) {
    $bucketno_expr = 'Result.classid';
  }
} else {
  $and_roundid_ok = '';
  $keyfield = 'RaceChart.roundid';
  $key = $roundid;
  if ($bucketed) {
    $bucketno_expr = 'RegistrationInfo.rankid';
  }
}

// Rounds(roundid, round, classid)
// Roster(rosterid, roundid, classid, racerid)
// RaceChart(resultid, classid, roundid, heat, lane, racerid, finishtime, completed)
// RegistrationInfo(racerid, ..., classid, rankid)
$stmt = $db->prepare('SELECT Result.racerid, '.$bucketno_expr.' AS bucketno, score'
                     .' FROM (SELECT racerid, Rounds.classid, '
                     .sql_scoring_expression('finishtime', 'finishplace').' AS score'
                     .'       FROM RaceChart'
                     .'       INNER JOIN Rounds'
                     .'       ON RaceChart.roundid = Rounds.roundid'
                     .'       WHERE '.$keyfield.' = :key'
                     .$and_roundid_ok
					 // This is basically "GROUP BY racerid"; racerid
					 // (should) determine Rounds.classid uniquely.
                     .'       GROUP BY racerid, Rounds.classid) AS Result'
                     .' INNER JOIN RegistrationInfo'
                     .'  ON Result.racerid = RegistrationInfo.racerid'
                     .' ORDER BY score');
$stmt->execute(array(':key' => $key));
foreach ($stmt as $row) {
  $bucketno = $row['bucketno'];
  if (!isset($buckets[$bucketno])) {
    $buckets[$bucketno] = array();
  }
  $n_in_bucket = count($buckets[$bucketno]);
  if ($n_in_bucket < $bucket_limit ||
      // If there's a tie for third when picking top 3, include additional finalists
      $buckets[$bucketno][$n_in_bucket - 1]['score'] >= $row['score']) {
    echo "<finalist racerid=\"".$row['racerid']."\" bucket_number=\"".$bucketno."\"/>\n";
    $buckets[$bucketno][] = array('racerid' => $row['racerid'], 'score' => $row['score']);
  }
}
$stmt->closeCursor();

$stmt = $db->prepare('INSERT INTO Roster(roundid, classid, racerid)'
                     .' VALUES(:roundid, :classid, :racerid)');
foreach ($buckets as $bucket) {
  foreach ($bucket as $racer) {
    $stmt->execute(array(':roundid' => $new_roundid,
                         ':classid' => $classid,
                         ':racerid' => $racer['racerid']));
  }
}

echo "<new-round roundid=\"".$new_roundid."\"/>\n";
echo "<success/>\n";

require('ajax/query.poll.coordinator.inc');

end_response();

?>
