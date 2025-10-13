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
  record_action(array('action' => 'round.*create',
                      'roundid' => $roundid,
                      'classid' => $classid,
                      'round' => $roundno));
  record_event(EVENT_ROUND_ADDED, array('roundid' => $roundid,
                                        'classid' => $classid,
                                        'other' => 'round '.$roundno));
  return $roundid;
}


function calculate_roster_from_classes($classids, $bucketed, $bucket_limit) {
  $standings = new StandingsOracle();
  list($classes, $classseq,  $ranks, $rankseq) = classes_and_ranks();

  $bucket_keys = array();
  $bucket_names = array();
  foreach ($classids as $cl) {
    $bucket_keys[] = $standings->class_key($cl);
    $bucket_names[] = $classes[$cl]['class'];
  }

  return calculate_roster_from_keys($bucket_keys, $bucket_names,
                                    $bucketed ? 1 : 0, $bucket_limit, $standings);
}

function calculate_roster_from_subgroups($rankids, $bucketed, $bucket_limit) {
  $standings = new StandingsOracle();
  list($classes, $classseq,  $ranks, $rankseq) = classes_and_ranks();

  $bucket_keys = array();
  $bucket_names = array();
  foreach ($rankids as $rankid) {
    $bucket_keys[] = $standings->rank_key($rankid);
    $bucket_names[] = $ranks[$rankid]['rank'];
  }

  return calculate_roster_from_keys($bucket_keys, $bucket_names,
                                    $bucketed ? 1 : 0, $bucket_limit, $standings);
}

// For a follow-on round to an existing racing round ("organic" roster), 
function calculate_roster_follow_on_round($classid, $bucketed, $bucket_limit) {
  global $db;

  if (!$bucketed) {
    return calculate_roster_from_classes(array($classid), $bucketed, $bucket_limit);
  }
  
  $stmt = $db->prepare('SELECT rankid FROM Ranks WHERE classid = :classid');
  $stmt->execute(array(':classid' => $classid));
  $rankids = $stmt->fetchAll(PDO::FETCH_COLUMN, 0);

  return calculate_roster_from_subgroups($rankids, $bucketed, $bucket_limit);
}


// $bucket_keys: An array of standings key symbols
// $bucket_names: An array of names for the buckets, parallel to $bucket_keys
// $bucket_multiplier is 0 for no bucketing (take top overall), or 1 for bucketing by key
// $bucket_limit: How many racers per bucket.  May be exceeded for ties within a bucket.
// $standings: A StandingsOracle instance
//
// Returns two values: (1) an unordered list of trophy-eligible racers, capped
// to $bucket_limit racers from each bucket, and (2) a list of "concerns" about
// buckets that weren't fully filled.
function calculate_roster_from_keys($bucket_keys, $bucket_names, $bucket_multiplier,
                                    $bucket_limit, &$standings) {
  global $db;

  $buckets = array();
  while (count($buckets) < count($bucket_keys)) {
    $buckets[] = array();
  }

  foreach ($standings->result_summary as &$row) {
    foreach ($row['outcome'] as $key => $place) {
      if (($bucketno = array_search($key, $bucket_keys)) !== false) {
        $bucket = &$buckets[$bucketno * $bucket_multiplier];
        $n_in_bucket = count($bucket);
        if ($n_in_bucket < $bucket_limit ||
            $bucket[$n_in_bucket - 1]['score'] >= $row['score']) {
          $bucket[] = array('racerid' => $row['racerid'], 'score' => $row['score']);
        }
        break;
      }
    }
  }
  $finalists = array();

  // Some or all buckets may be smaller than anticipated/desired.
  // That may be benign (only two racers in one of the classes), or it may be confusing.
  // We'll report any of these as "concerns" about the roster.
  $concerns = array();

  for ($b_index = 0; $b_index < count($buckets); ++$b_index) {
    $b = $buckets[$b_index];
    if (count($b) < $bucket_limit) {
      if (count($b) == 0) {
        $concerns[] = 'There are no eligible '.$bucket_names[$b_index].' racer(s).';
      } else {
        $concerns[] = 'There are only '.count($b).' eligible '.$bucket_names[$b_index].' racer(s).';
      }
    }
    foreach ($b as $racer) {
      $finalists[] = array('racerid' => $racer['racerid'] + 0,
                           'score' => $racer['score'] + 0);
    }
  }
  return array($finalists, $concerns);
}

function write_roster_entries($new_roundid, $classid, &$finalists) {
  global $db;
  $stmt = $db->prepare('INSERT INTO Roster(roundid, classid, racerid)'
                       .' VALUES(:roundid, :classid, :racerid)');
  foreach ($finalists as $racer) {
    $stmt->execute(array(':roundid' => $new_roundid,
                         ':classid' => $classid,
                         ':racerid' => $racer['racerid']));
  }
}

?>
