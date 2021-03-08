<?php

require_once('inc/standings.inc');
require_once('inc/events.inc');

function make_new_round($classid, $roundno) {
  global $db;
  $stmt = $db->prepare('INSERT INTO Rounds(classid, round, phase, charttype)'
                       .' VALUES(:classid, :roundno, 0, 0)');
  $stmt->execute(array(':classid' => $classid,
                       ':roundno' => $roundno));
  $roundid = read_single_value('SELECT roundid FROM Rounds'
                               .' WHERE round = :roundno'
                               .'  AND classid = :classid',
                               array(':classid' => $classid,
                                     ':roundno' => $roundno));
  record_event_create_round($roundid, $classid, $roundno);
  return $roundid;
}

define('ROSTER_UNBUCKETED', 0);
define('ROSTER_BUCKETED_ORGANIC', 1);
define('ROSTER_BUCKETED_AGGREGATE', 2);
define('ROSTER_BUCKETED_AGGREGATE_BY_SUBGROUP', 3);

// $bucketing: one of the enum constants above.
// $new_roundid: roundid for the round being populated.
// $classid: classid for the round's class
// $bucket_limit: number of racers to choose in each bucket (however determined)
// $constituent_roundids: array of roundids
// $rankids: array of rankids for aggregate-round-by-subgroup
function populate_roster($bucketing, $new_roundid, $classid, $bucket_limit,
                         $constituent_roundids, $rankids) {
  global $db;

  $buckets = array();  // bucketno => array of { racerid , score }

  if (count($rankids) == 0) {
    $and_filter = '';
  } else {
    $and_filter = 'AND RegistrationInfo.rankid IN ('.implode(',', $rankids).')';
  }
  if ($bucketing == ROSTER_UNBUCKETED) {
    // If top n across the whole unit or den, bucketno is a constant (1)
    $bucketno_expr = '1';
  } else if ($bucketing == ROSTER_BUCKETED_ORGANIC) {
    $bucketno_expr = 'RegistrationInfo.rankid';
  } else if ($bucketing == ROSTER_BUCKETED_AGGREGATE) {
    $bucketno_expr = 'Result.roundid';
  } else /* ROSTER_BUCKETED_AGGREGATE_BY_SUBGROUP */ {
    $bucketno_expr = 'RegistrationInfo.rankid';
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
                       .'       WHERE RaceChart.roundid IN ('.implode(',', $constituent_roundids).')'
                       .' AND (finishtime IS NOT NULL OR finishplace IS NOT NULL)'
                       // This is basically "GROUP BY racerid"; racerid will
                       // determine Rounds.roundid uniquely, unless a racer is
                       // part of two different constituent rounds; in that case,
                       // we'll treat that racer like two different racers.
                       .'       GROUP BY racerid, Rounds.roundid) AS Result'
                       .' INNER JOIN RegistrationInfo'
                       .'  ON Result.racerid = RegistrationInfo.racerid'
                       .' WHERE passedinspection = 1 AND exclude = 0 '
                       .$and_filter
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
      echo "<finalist racerid=\"$row[racerid]\" bucket_number=\"".$bucketno."\" score=\"$row[score]\"/>\n";
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
}

?>
