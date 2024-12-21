<?php
// $_POST roundid, classid_<nnn>, rankid_<nnn> classname, top, bucketed

// Populates a new round by selecting top finishers from a previous round or rounds.

// If $_POST['roundid'] is not empty, it identifies a completed round, and
// implies that we're populating a new follow-on round for the same class, based
// on the top finishers from the identified round.
//
// If roundid is empty, then we're generating an aggregate round, taking top
// finishers across multiple previous rounds.  There are two ways this can happen:
//
// - If $_POST['classname'] is present, then we're creating an aggregate round for
//    a new aggregate class.  $_POST['classname'] will be taken as the name of
//    the new class.
//
//    The classes whose results feed into the new aggregate round are identified
//    by $_POST keys of the form $_POST['classid_<nnn>'].
//
//    For an "aggregate round by subgroup", we need two or more subgroups, posted
//    as $_POST['rankid_<nnn>'].  For each requested subgroup, we look at the
//    highest-numbered round for the corresponding (organic) class.
//
//  - If $_POST['classid'] is present, then we're creating an aggregate round
//    based on a defined aggregate class.  The rounds whose results feed into
//    the new aggregate round will be the highest defined round for each of the
//    aggregate class' constituent classes.
//
// $_POST['top'] identifies how many racers to select for the new round, but its
// interpretation also depends on $_POST['bucketed'].
//
// For an aggregate round, if $POST['bucketed'] is true, then depending on how
// the aggregate is formed, $_POST['top'] tells how many racers to choose from
// either each of the constituent classes, or each of the constituent subgroups.
//
// For a non-aggregate round, if $_POST['bucketed'] is true, then $_POST['top']
// tells how many racers to choose from each subgroup.
//
// (Note that this ignores any racing aggregate rounds that might draw racers
// from the requested subgroups.)


require_once('inc/classes.inc');
require_once('inc/newracer.inc');
require_once('inc/aggregate_round.inc');
require_once('inc/roster.inc');
require_once('inc/rounds.inc');


// With $new_round and $classid settled, make the new round (unless it
// already exists)
function prepare_new_round($classid, $new_round) {
  global $db;
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
    $new_roundid = make_new_round($classid, $new_round);
  }
  return $new_roundid;
}


$bucket_limit = $_POST['top'];
$bucketed = isset($_POST['bucketed']) && $_POST['bucketed'];
$rankids = array();

if (isset($_POST['roundid']) && strlen($_POST['roundid']) > 0) {
  $roundid = $_POST['roundid'];
  $constituent_roundids = array($roundid);
  list($classid, $old_round) = read_single_row('SELECT classid, round FROM Rounds'
                                               .' WHERE roundid = :roundid',
                                               array(':roundid' => $roundid));
  // TODO: Handle not-found case
  $new_roundid = prepare_new_round($classid, 1 + $old_round);
  $finalists = calculate_roster_follow_on_round($classid, $bucketed, $bucket_limit);
} else {
  if (isset($_POST['classname'])) {
    list($classes, $classseq, $ranks, $rankseq) = classes_and_ranks();
    $classids = array();
    $rankids = array();
    foreach ($_POST as $key => $value) {
      if (substr($key, 0, strlen('classid_')) == 'classid_') {
        $classids[] = substr($key, strlen('classid_'));
      } else if (substr($key, 0, strlen('rankid_')) == 'rankid_') {
        $r = substr($key, strlen('rankid_'));
        $rankids[] = $r;
        $classids[] = $ranks[$r]['classid'];
      }
    }
    $classids = array_unique($classids);

    $classid = create_aggregate_class($_POST['classname'], $classids, $rankids, false);
    // When creating a new aggregate class as part of creating an aggregate
    // round, we assume that this is a "Grand Finals" round to determine overall
    // pack champions.  This choice can still be undone from the awards
    // presentation screen if it's not the intent.
    write_raceinfo('full-field-calc', $classid);
  } else {
    $classid = $_POST['classid'];
  }

  $new_roundid = prepare_new_round($classid, 1);

  $details = read_single_row('SELECT'
                             .(schema_version() >= 3 ? ' constituents,' : ' \'\' AS constituents,')
                             .(schema_version() >= 5 ? ' rankids' : ' \'\' AS rankids,')
                             .' FROM Classes'
                             .' WHERE classid = :classid',
                             array(':classid' => $classid),
                             PDO::FETCH_ASSOC);

  if (empty($details['rankids'])) {
    $finalists = calculate_roster_from_classes(
      array_filter(explode(',', $details['constituents']), 'strlen'), $bucketed, $bucket_limit);
  } else {
    $finalists = calculate_roster_from_subgroups(
      array_filter(explode(',', $details['rankids']), 'strlen'), $bucketed, $bucket_limit);
  }
}

json_out('finalists', $finalists);

write_roster_entries($new_roundid, $classid, $finalists);

if (count($finalists) == 0) {
  json_failure('no_racers', 'No eligible racers found.');
} else {
  json_out('new-round', array('roundid' => $new_roundid));
  json_success();

  require('ajax/query.poll.coordinator.inc');
}

?>
