<?php

// $_POST['racerid']
// $_POST['awardname']
//
// If awardname is blank, any ad hoc award for this racer is removed.

if (have_permission(JUDGING_PERMISSION)) {
  start_response();

  // Remove old award, if any
  $stmt = $db->prepare("DELETE FROM Awards WHERE racerid = :racerid");
  $stmt->execute(array(':racerid' => $_POST['racerid']));

  // Write new award, if not empty
  if ($_POST['awardname']) {
    // Populating the classid and rankid fields for the award will make the ad
    // hoc awards show up when presenting either the full pack or any group or
    // rank within the pack.  If they're left empty, then they're only
    // pack-level awards, and get masked out at the group level.
    list($classid, $rankid) = read_single_row('SELECT classid, rankid FROM RegistrationInfo'
                                              .' WHERE racerid = :racerid',
                                              array(':racerid' => $_POST['racerid']));
    $stmt = $db->prepare("INSERT INTO Awards(awardname, awardtypeid, racerid, classid, rankid, sort)"
                         ." VALUES(:awardname,"
                         ." (SELECT awardtypeid FROM AwardTypes WHERE awardtype = 'Ad Hoc'),"
                         ." :racerid, :classid, :rankid, 999)");
    $stmt->execute(array(':racerid' => $_POST['racerid'],
                         ':classid' => $classid,
                         ':rankid' => $rankid,
                         ':awardname' => $_POST['awardname']));
  }

  echo "<success/>\n";
  require('ajax/query.award.list.inc');

  end_response();
} else {
  not_authorized();
}
?>