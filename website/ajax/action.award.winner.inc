<?php
/*
 Required: awardid ('new' for new award)
  racerid
*/
require_once('inc/awards-xml.inc');

if (have_permission(JUDGING_PERMISSION)) {
  start_response();

  try {
    $ok = true;
    if ($_POST['racerid'] != 0) {
      $award = read_single_row('SELECT classid, rankid FROM Awards WHERE awardid = :awardid',
                               array(':awardid' => $_POST['awardid']));
      $racer = read_single_row('SELECT classid, rankid, exclude FROM RegistrationInfo WHERE racerid = :racerid',
                               array(':racerid' => $_POST['racerid']));
      if (!$award) {
        $ok = false;
        echo "<failure code='no-such-award'>Award not found</failure>\n";
      } else if (!$racer) {
        $ok = false;
        echo "<failure code='no-such-racer'>Racer not found</failure>\n";
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
              echo "<rank-eligible>$award_rankid $racer_rankid ".json_encode($agg)."</rank-eligible>\n";  // TODO
              $award_rankid = 0;
            } else {
              echo "<rank-eligible>FAIL $award_rankid $racer_rankid ".json_encode($agg['rankids'])."</rank-eligible>\n";  // TODO
              $award_rankid = -1;
            }
          }
        }

        if ($racer_exclude) {
          $ok = false;
          echo "<failure code='ineligible'>Racer not eligible for awards</failure>\n";
        } else if (isset($award_classid) && $award_classid != 0 && $award_classid != $racer_classid) {
          $ok = false;
          echo "<failure code='wrong-class'>Award is for a different class</failure>\n";
        } else if (isset($award_rankid) && $award_rankid != 0 && $award_rankid != $racer_rankid) {
          $ok = false;
          echo "<failure code='wrong-subgroup'>Award is for a different subgroup</failure>\n";
        }
      }
    }
    
    if ($ok) {
      $stmt = $db->prepare('UPDATE Awards SET racerid = :racerid WHERE awardid = :awardid');
      $stmt->execute(array(':awardid' => $_POST['awardid'],
                           ':racerid' => $_POST['racerid']));
      echo "<success/>\n";
      emit_award_list(/* include_ad_hoc */ true);
    }
  } catch (Exception $e) {
    echo "<failure code='sql'>".htmlspecialchars($e->getMessage(), ENT_QUOTES, 'UTF-8')."</failure>\n";
  }
  end_response();
} else {
  not_authorized();
}
?>