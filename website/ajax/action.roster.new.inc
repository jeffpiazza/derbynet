<?php
// $_POST roundid, roundid_<nnn>, classname, top, bucketed

// Populates a new round by selecting top finishers from a previous round or rounds.

// If $_POST['roundid'] is not empty, it identifies a completed round, and
// implies that we're populating a new follow-on round for the same class, based
// on the top finishers from the identified round.
//
// If roundid is empty, then we're generating an aggregate round, taking top
// finishers across multiple previous rounds.  Those previous rounds are
// identified by $_POST keys of the form roundid_<nnn>.  A new class will be
// created for the round; $_POST['classname'] will be taken as the name of the
// new class.
//
// "top" identifies how many racers to select for the new round, but its interpretation also
// depends on "bucketed".
//
// For an aggregate round, if "bucketed" is true, then "top" tells how many
// racers to choose from each of the prior rounds feeding in to the new round.
//
// For a non-aggreate round, if "bucketed" is true, then "top" tells how
// many racers to choose from each subgroup.
//
// TODO: If racing isn't complete, selection of top finishers is likely to be wrong.

start_response();

require_once('inc/standings.inc');
require_once('inc/newracer.inc');
require_once('inc/aggregate_round.inc');

$bucket_limit = $_POST['top'];
$bucketed = isset($_POST['bucketed']) && $_POST['bucketed'];

if (isset($_POST['roundid']) && strlen($_POST['roundid']) > 0) {
  $aggregate = false;
  $roundid = $_POST['roundid'];
  list($classid, $old_round) = read_single_row('SELECT classid, round FROM Rounds'
                                               .' WHERE roundid = :roundid',
                                               array(':roundid' => $roundid));
  // TODO: Handle not-found case
  $new_round = 1 + $old_round;
} else {
  $aggregate = true;
  $all_roundids = '0';
  foreach ($_POST as $key => $value) {
    if (substr($key, 0, strlen('roundid_')) == 'roundid_') {
      $all_roundids .= ', '.substr($key, strlen('roundid_'));
    }
  }
  // TODO: Handle 0 or 1 roundid's provided

  // A new aggregate round creates a new pseudo-class for the aggregate, and
  // starts at round 1
  $new_round = 1;

  $classid = create_aggregate_class($_POST['classname'], $all_roundids, false);
}

// With $new_round and $classid settled, make the new round (unless it
// already exists)

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
// If bucketed, bucketno is rankid (for a den) or classid (for aggreate).
$bucketno_expr = '1';

if ($aggregate) {
  if ($bucketed) {
    $bucketno_expr = 'Result.roundid';
  }
} else {
  $all_roundids = $roundid;
  if ($bucketed) {
    $bucketno_expr = 'RegistrationInfo.rankid';
  }
}

// Rounds(roundid, round, classid)
// Roster(rosterid, roundid, classid, racerid)
// RaceChart(resultid, classid, roundid, heat, lane, racerid, finishtime, completed)
// RegistrationInfo(racerid, ..., classid, rankid)
$stmt = $db->prepare('SELECT Result.racerid, '.$bucketno_expr.' AS bucketno, score'
                     .' FROM (SELECT racerid, Rounds.roundid, '
                     .sql_scoring_expression('finishtime', 'finishplace').' AS score'
                     .'       FROM RaceChart'
                     .'       INNER JOIN Rounds'
                     .'       ON RaceChart.roundid = Rounds.roundid'
                     .'       WHERE RaceChart.roundid IN ('.$all_roundids.')'
                     .' AND (finishtime IS NOT NULL OR finishplace IS NOT NULL)'
					 // This is basically "GROUP BY racerid"; racerid will
					 // determine Rounds.roundid uniquely, unless a racer is
					 // part of two different constituent rounds; in that case,
					 // we'll treat that racer like two different racers.
                     .'       GROUP BY racerid, Rounds.roundid) AS Result'
                     .' INNER JOIN RegistrationInfo'
                     .'  ON Result.racerid = RegistrationInfo.racerid'
                     .' WHERE passedinspection = 1 AND exclude = 0'
                     .' ORDER BY score');
$stmt->execute(array());
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
