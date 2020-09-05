<?php
// $_POST roundid, roundid_<nnn>, classname, top, bucketed

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
//    the new class.  The rounds whose results feed into the new aggregate round
//    are identified by $_POST keys of the form roundid_<nnn>.
//
//  - If $_POST['classid'] is present, then we're creating an aggregate round
//    based on a defined aggregate class.  The rounds whose results feed into
//    the new aggregate round will be the highest defined round for each of the
//    aggregate class' constituent classes.
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

require_once('inc/newracer.inc');
require_once('inc/aggregate_round.inc');
require_once('inc/roster.inc');

$bucket_limit = $_POST['top'];
$bucketed = isset($_POST['bucketed']) && $_POST['bucketed'];

if (isset($_POST['roundid']) && strlen($_POST['roundid']) > 0) {
  $aggregate = false;
  $roundid = $_POST['roundid'];
  $constituent_roundids = $roundid;
  list($classid, $old_round) = read_single_row('SELECT classid, round FROM Rounds'
                                               .' WHERE roundid = :roundid',
                                               array(':roundid' => $roundid));
  // TODO: Handle not-found case
  $new_round = 1 + $old_round;
} else {
  $aggregate = true;

  // A new aggregate round starts at round 1 and may create a new pseudo-class
  // for the aggregate.
  $new_round = 1;

  if (isset($_POST['classname'])) {
    $constituent_roundids = '0';
    foreach ($_POST as $key => $value) {
      if (substr($key, 0, strlen('roundid_')) == 'roundid_') {
        $constituent_roundids .= ', '.substr($key, strlen('roundid_'));
      }
    }
    $classid = create_aggregate_class($_POST['classname'], $constituent_roundids, false);
    // TODO: Handle 0 or 1 roundid's provided
  } else {
    $classid = $_POST['classid'];
    // TODO Worry about schema version?
    $constituents = read_single_value('SELECT constituents FROM Classes'
                                      .' WHERE classid = :classid',
                                      array(':classid' => $classid));
    $stmt = $db->prepare('SELECT roundid FROM Rounds r'
                         .' WHERE classid IN ('.$constituents.')'
                         .'  AND round = (SELECT MAX(round) FROM Rounds r2'
                         .'               WHERE r2.classid = r.classid)');
    $stmt->execute(array());
    $constituent_roundids = implode(',', $stmt->fetchAll(PDO::FETCH_COLUMN, 0));
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
  $new_roundid = make_new_round($classid, $new_round);
}

populate_roster($new_roundid, $classid, $constituent_roundids, $bucketed, $bucket_limit, $aggregate);

echo "<new-round roundid=\"".$new_roundid."\"/>\n";
echo "<success/>\n";

require('ajax/query.poll.coordinator.inc');

end_response();

?>
