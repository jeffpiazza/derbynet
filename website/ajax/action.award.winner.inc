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
      $racer = read_single_row('SELECT classid, rankid FROM RegistrationInfo WHERE racerid = :racerid',
                               array(':racerid' => $_POST['racerid']));
      if (!$award) {
        $ok = false;
        echo "<failure code='no-such-award'>Award not found</failure>\n";
      } else if (!$racer) {
        $ok = false;
        echo "<failure code='no-such-racer'>Racer not found</failure>\n";
      } else {
        list($award_classid, $award_rankid) = $award;
        list($racer_classid, $racer_rankid) = $racer;
        if (isset($award_classid) && $award_classid != 0 && $award_classid != $racer_classid) {
          $ok = false;
          echo "<failure code='wrong-class'>Award is for a different class</failure>\n";
        } else if (isset($award_rankid) && $award_rankid != 0 && $award_rankid != $racer_rankid) {
          $ok = false;
          echo "<failure code='wrong-class'>Award is for a different subgroup</failure>\n";
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