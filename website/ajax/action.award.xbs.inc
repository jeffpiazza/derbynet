<?php
if ($_POST['value']) {
  take_action_if_permitted('INSERT INTO Awards(AwardName, AwardTypeID, RacerID)'
                           .' VALUES(:award, 3, :racerid)',
                           array(':award' => read_raceinfo('xbs-award'),
                                 ':racerid' => $_POST['racer']),
                           CHECK_IN_RACERS_PERMISSION);
} else {
  take_action_if_permitted('DELETE FROM Awards WHERE racerid = :racerid',
                           array(':racerid' => $_POST['racer']),
                           CHECK_IN_RACERS_PERMISSION);
}
?>