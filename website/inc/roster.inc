<?php

require_once('inc/standings.inc');

function make_new_round($classid, $roundno) {
  global $db;
  $stmt = $db->prepare('INSERT INTO Rounds(classid, round, phase, charttype)'
                       .' VALUES(:classid, :roundno, 0, 0)');
  $stmt->execute(array(':classid' => $classid,
                       ':roundno' => $roundno));
  return read_single_value('SELECT roundid FROM Rounds'
                           .' WHERE round = :roundno'
                           .'  AND classid = :classid',
                           array(':classid' => $classid,
                                 ':roundno' => $roundno));
}

function populate_roster($new_roundid, $classid, $constituent_roundids,
                         $bucketed, $bucket_limit, $is_aggregate) {
  global $db;

  $buckets = array();  // bucketno => array of { racerid , score }

  // If top n across the whole unit or den, bucketno is a constant (1)
  // If bucketed, bucketno is rankid (for a den) or classid (for aggreate).
  $bucketno_expr = '1';

  if ($bucketed) {
    if ($is_aggregate) {
      $bucketno_expr = 'Result.roundid';
    } else {
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
                       .'       WHERE RaceChart.roundid IN ('.$constituent_roundids.')'
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
