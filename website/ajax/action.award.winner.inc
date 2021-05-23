<?php
/*
 Required: awardid ('new' for new award)
  racerid
*/
require_once('inc/json-awards.inc');

if (have_permission(JUDGING_PERMISSION)) {
  try {
    $ok = true;
    if ($_POST['racerid'] != 0) {
      $award = read_single_row('SELECT classid, rankid FROM Awards WHERE awardid = :awardid',
                               array(':awardid' => $_POST['awardid']));
      $racer = read_single_row('SELECT classid, rankid, exclude FROM RegistrationInfo WHERE racerid = :racerid',
                               array(':racerid' => $_POST['racerid']));
      if (!$award) {
        $ok = false;
        json_failure('no-such-award', "Award not found");
      } else if (!$racer) {
        $ok = false;
        json_failure('no-such-racer', "Racer not found");
      } else {
        list($award_classid, $award_rankid) = $award;
        list($racer_classid, $racer_rankid, $racer_exclude) = $racer;

        if (isset($award_classid) && $award_classid != 0) {
          $agg_classes = aggregate_classes();
          if (isset($agg_classes[$award_classid])) {
            compute_aggregate_constituents_tc($agg_classes);
            $agg = $agg_classes[$award_classid];
            if (in_array($racer_classid, $agg['constituents_tc'])) {
              // Racer is a member of one of the constituent classes, and so is
              // eligible for the award.  Express this by removing the award
              // class restriction that gets tested below.
              $award_classid = 0;
            }
            if (count($agg['rankids']) == 0 || in_array($racer_rankid, $agg['rankids'])) {
              $award_rankid = 0;
            } else {
              $award_rankid = -1;
            }
          }
        }

        if ($racer_exclude) {
          $ok = false;
          json_failure('ineligible', "Racer not eligible for awards");
        } else if (isset($award_classid) && $award_classid != 0 && $award_classid != $racer_classid) {
          $ok = false;
          json_failure('wrong-class', "Award is for a different class");
        } else if (isset($award_rankid) && $award_rankid != 0 && $award_rankid != $racer_rankid) {
          $ok = false;
          json_failure('wrong-subgroup', "Award is for a different subgroup");
        }
      }
    }
    
    if ($ok) {
      $stmt = $db->prepare('UPDATE Awards SET racerid = :racerid WHERE awardid = :awardid');
      $stmt->execute(array(':awardid' => $_POST['awardid'],
                           ':racerid' => $_POST['racerid']));
      json_success();
      json_out_awards(true);
    }
  } catch (Exception $e) {
    json_failure('sql', $e->getMessage());
  }
} else {
  json_not_authorized();
}
?>