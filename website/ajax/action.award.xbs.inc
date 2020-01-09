<?php
$award_name = read_raceinfo('xbs-award', 'Exclusively By Scout');

if ($_POST['value']) {
  take_action_if_permitted('INSERT INTO Awards(awardname, awardtypeid, racerid)'
                           .' VALUES(:award, 3, :racerid)',
                           array(':award' => $award_name,
                                 ':racerid' => $_POST['racer']),
                           CHECK_IN_RACERS_PERMISSION);
} else {
  take_action_if_permitted('DELETE FROM Awards WHERE racerid = :racerid'
                           .' AND awardname = :award',
                           array(':award' => $award_name,
                                 ':racerid' => $_POST['racer']),
                           CHECK_IN_RACERS_PERMISSION);
}
?>