<?php @session_start(); ?>
<?php

// Receive POSTs from the checkin page to perform check-in actions,
// return XML responses

// TODO: Single quotes aren't escaped properly for MS Access!!  Beware
// editing a name like O'Reilly!

require_once('data.inc');
require_once('permissions.inc');
require_once('authorize.inc');


header('Content-Type: text/xml');

function xml_for_reload() {
  return '<reload>1</reload>';
}

function take_action_silently($sql) {
  global $conn;
  $log_sql = $sql;
  if (strlen($log_sql) > 250)
	$log_sql = substr($log_sql, 1, 120) . '...' . substr($log_sql, strlen($log_sql) - 120);
  // MS Access quotes single-quotes within single-quotes by doubling,
  // not by prefixing with backslash, so addslashes is insufficient.
  // The str_replace gets us through this, but there's a more general problem lurking here...
  @odbc_exec($conn, 'INSERT INTO CheckinAudit(tstamp, stmt) VALUES(NOW(), \''
			        .str_replace('\'', '`', addslashes($log_sql)).'\')');

  if (odbc_exec($conn, $sql) === false) {
	return false;
  } else {
	return true;
  }
}

function take_action($sql, $body_xml = '') {
  global $conn;
  if (take_action_silently($sql)) {
	echo '<checkin><success/>'.$body_xml.'</checkin>';
  } else {
	echo '<checkin><failure>'.$sql.' failed: '.odbc_errormsg($conn).' [EOM]</failure></checkin>';
  }
}

function not_authorized() {
  echo "<checkin><failure>Not authorized -- please see race coordinator</failure></checkin>";
}

function take_action_if_permitted($sql, $permission, $body_xml = '') {
  if (have_permission($permission)) {
	take_action($sql, $body_xml);
  } else {
	not_authorized();
  }
}

if ($_POST['action'] == 'pass') {
    $ok_to_change = true;

    // Protect against changes to a racer who already has schedule data.
	if (0 + read_single_value('SELECT COUNT(*) FROM RaceChart WHERE RacerID = '.$_POST['racer'], 0))
      $ok_to_change = false;

    if ($ok_to_change) {
		if ($_POST['value'] || have_permission(REVERT_CHECK_IN_PERMISSION)) {
		  take_action('UPDATE RegistrationInfo SET PassedInspection = '.$_POST['value']
					  .' WHERE RacerID = '.$_POST['racer']);
		} else {
		  not_authorized();
		}
    } else {
      // Force reload of the page to show schedule data
      echo '<checkin><failure>Schedule data already exists for this racer</failure><reload/></checkin>';
    }
} else if ($_POST['action'] == 'xbs') {
    if ($_POST['value']) {
	  take_action('INSERT INTO Awards(AwardName, AwardTypeID, RacerID)'
				  .' values(\''.addslashes(read_raceinfo('xbs-award')).'\', 3, '
				  .$_POST['racer'].')');
    } else {
      take_action('DELETE FROM Awards WHERE RacerID = '.$_POST['racer']);
    }
} else if ($_POST['action'] == 'renumber') {
  $passed_inspection = read_single_value('SELECT PassedInspection OR CarNumber = 99999'
										 .' FROM RegistrationInfo'
										 .' WHERE RacerID = '.$_POST['racer']);
  take_action_if_permitted('UPDATE RegistrationInfo'
						   .' SET PassedInspection = PassedInspection OR CarNumber = 99999,'
						   .' CarNumber = '.$_POST['value']
						   .' WHERE RacerID = '.$_POST['racer'],
						   RENUMBER_CAR_PERMISSION,
						   '<passed racer="'.$_POST['racer'].'">'
						   .$passed_inspection.'</passed>');
} else if ($_POST['action'] == 'classchange') {
  if (have_permission(CHANGE_RACER_RANK_PERMISSION)) {
	$racerid = $_POST['racer'];
	$rankid = $_POST['value'];
	$classid = read_single_value('SELECT ClassID FROM Ranks WHERE RankID = '.$rankid);
	if (!$classid) {
	  echo '<checkin><failure>['.$fetch.']</failure></checkin>';
	  exit();
	}
	$new_roundid = read_single_value('SELECT RoundID FROM Rounds WHERE Round = 1 AND ClassID = '.$classid);
	$old_classid = read_single_value('SELECT ClassID FROM RegistrationInfo WHERE RacerID = '.$racerid);
	$old_roundid = read_single_value('SELECT RoundID FROM Rounds Where Round = 1'
									 .' AND ClassID = '.$old_classid);
	if (take_action_silently('UPDATE RegistrationInfo SET RankID = '.$rankid
							 .', ClassID = '.$classid
							 .' WHERE RacerID = '.$racerid)) {
	  take_action('UPDATE Roster SET RoundID = '.$new_roundid.' WHERE RacerID = '.$racerid
				  .' AND RoundID = '.$old_roundid,
				xml_for_reload());
    }
  } else {
	not_authorized();
  }
} else if ($_POST['action'] == 'photo') {
  if ($_POST['previous'])
	// TODO: Something is writing NULLs into ImageFile field.  Does
	// Access treat an empty string as NULL?
	take_action_silently('UPDATE RegistrationInfo SET ImageFile = \'\''
						 .' WHERE RacerID = '.$_POST['previous']);
  take_action('UPDATE RegistrationInfo set ImageFile = \''.$_POST['photo']
			  .'\' WHERE RacerID = '.$_POST['racer']);
} else if ($_POST['action'] == 'initaudit') {
  take_action('CREATE TABLE CheckinAudit ('
			  .'   seq    COUNTER,'
			  .'   stmt   VARCHAR (250),'
			  .'   tstamp DATETIME'
			  .')');
} else if ($_POST['action'] == 'initnumbers') {
  // For a number of races I've been involved in, car numbers are
  // assigned at check-in, and it's fairly easy to forget to click
  // "passed" when you're fiddling with entering the car number for a
  // boy.  For this kind of race, the 99999 value gets noticed in the
  // renumber action, above, as a signal that renumberig should also
  // pass the inspection.
  take_action('UPDATE RegistrationInfo SET CarNumber = 99999 WHERE PassedInspection = 0');
} else {
    echo '<checkin><failure>Unrecognized post: '.$_POST['action'].'</failure></checkin>';
}
  
  odbc_close($conn);
?>