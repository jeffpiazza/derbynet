<?php
if (have_permission(CHANGE_RACER_RANK_PERMISSION)) {
	$racerid = $_POST['racer'];
	$rankid = $_POST['value'];
	$classid = read_single_value('SELECT classid FROM Ranks WHERE rankid = '.$rankid);
	if (!$classid) {
	  echo '<checkin><failure>['.$fetch.']</failure></checkin>';
	  exit();
	}
	$new_roundid = read_single_value('SELECT roundid FROM Rounds WHERE round = 1 AND classid = '.$classid);
	$old_classid = read_single_value('SELECT classid FROM RegistrationInfo WHERE racerid = '.$racerid);
	$old_roundid = read_single_value('SELECT roundid FROM Rounds Where round = 1'
									 .' AND classid = '.$old_classid);
	if (take_action_silently('UPDATE RegistrationInfo SET rankid = :rankid'
							 .', classid = :classid'
							 .' WHERE racerid = :racerid'
							 array(':rankid' => $rankid,
								   ':classid' => $classid,
								   ':racerid' => $racerid))) {
		take_action('UPDATE Roster SET roundid = :new_roundid'
					.' WHERE racerid = :racerid'
					.' AND roundid = :old_roundid',
					array(':new_roundid' => $new_roundid,
						  ':racerid' => $racerid,
						  ':old_roundid' => $old_roundid),
				xml_for_reload());
    }
  } else {
	not_authorized();
  }
?>