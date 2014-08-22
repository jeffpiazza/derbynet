<?php
// $_POST roundid, top, bucketed

// TODO: Finalists are selected by average time, which may give the
// wrong result if some racers didn't finish all their heats.

start_response();

$bucket_limit = $_POST['top'];
$bucketed = isset($_POST['bucketed']) && $_POST['bucketed'];

if (isset($_POST['roundid'])) {
  $grand_finals = false;
  $roundid = $_POST['roundid'];
  list($classid, $old_round) = read_single_row('SELECT classid, round FROM Rounds WHERE roundid = :roundid',
                                         array(':roundid' => $roundid));
  $new_round = 1 + $old_round;
} else {
  $grand_finals = true;
  $old_round = read_single_value('SELECT MAX(round)'
                                 .' FROM RaceChart'
                                 .' INNER JOIN Rounds'
                                 .' ON RaceChart.roundid = Rounds.roundid'
                                 .' WHERE finishtime IS NOT NULL',
                                 array());
  $new_round = $old_round;
  $classid = read_single_value("SELECT classid FROM Classes WHERE class = 'Grand Finals'",
                               array(), 0);
  if (!$classid) {
    $db->exec("INSERT INTO Classes(class) VALUES('Grand Finals')");
    $classid = read_single_value("SELECT classid FROM Classes WHERE class = 'Grand Finals'",
                                 array(), 0);
  }
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
  $stmt = $db->prepare('INSERT INTO Rounds(round, classid) VALUES(:round, :classid)');
  $stmt->execute(array(':round' => $new_round,
                       ':classid' => $classid));
  $stmt->closeCursor();
  $new_roundid = read_single_value('SELECT roundid FROM Rounds'
                                   .' WHERE round = :round'
                                   .'  AND classid = :classid',
                                   array(':round' => $new_round,
                                         ':classid' => $classid));
}

$buckets = array();  // bucketno => array of racerids

// If top n across the whole unit or den, bucketno is a constant (1)
// If bucketed, bucketno is rankid (for a den) or classid (for grand finals).
$bucketno_expr = '1';

if (!$grand_finals) {
  $keyfield = 'RaceChart.roundid';
  $key = $roundid;
  if ($bucketed) {
    $bucketno_expr = 'Rounds.classid';
  }
} else {
  $keyfield = 'round';
  $key = $old_round;
  if ($bucketed) {
    $bucketno_expr = 'RegistrationInfo.rankid';
  }
}

// Rounds(roundid, round, classid)
// Roster(rosterid, roundid, classid, racerid)
// RaceChart(resultid, classid, roundid, heat, lane, racerid, finishtime, completed)
// RegistrationInfo(racerid, ..., classid, rankid)

$stmt = $db->prepare('SELECT Result.racerid, '.$bucketno_expr.' AS bucketno, total'
                     .' FROM (SELECT racerid, AVG(finishtime) AS total'
                     .'       FROM RaceChart'
                     .'       INNER JOIN Rounds'
                     .'       ON RaceChart.roundid = Rounds.roundid'
                     .'       WHERE '.$keyfield.' = :key'
                     .'       GROUP BY racerid) AS Result'
                     .' INNER JOIN RegistrationInfo'
                     .'  ON Result.racerid = RegistrationInfo.racerid'
                     .' ORDER BY total');
$stmt->execute(array(':key' => $key));
foreach ($stmt as $row) {
  $bucketno = $row['bucketno'];
  if (!isset($buckets[$bucketno])) {
    $buckets[$bucketno] = array();
  }
  if (count($buckets[$bucketno]) < $bucket_limit) {
    echo "<finalist racerid=\"".$row['racerid']."\" bucket_number=\"".$bucketno."\"/>\n";
    $buckets[$bucketno][] = $row['racerid'];
  } else {
    echo "<non-finalist racerid=\"".$row['racerid']."\" bucket_number=\"".$bucketno."\"/>\n";
  }
}
$stmt->closeCursor();

$stmt = $db->prepare('INSERT INTO Roster(roundid, classid, racerid)'
                     .' VALUES(:roundid, :classid, :racerid)');
foreach ($buckets as $bucket) {
  foreach ($bucket as $racerid) {
    $stmt->execute(array(':roundid' => $new_roundid,
                         ':classid' => $classid,
                         ':racerid' => $racerid));
  }
}

echo "<new-round roundid=\"".$new_roundid."\"/>\n";
echo "<success/>\n";

end_response();

?>