 <?php
// $_POST['who']:
//    'all' => all racers
//    'd' + partitionid => all members of a partition
//    'c' + classid => all members of a class
//    'r' + rankid => all members of one rank
//
// $_POST['what']:
//    'checkin' => set 'passed' value
//    'number' => assign car numbers
//    'eligibility' => change racer eligibility
//
// For what=checkin or what=eligible:
// $_POST['value']: 1 (make eligible or checked-in) or 0 (make ineligble/unchecked-in)
//
// For what=number:
// $_POST['start']: number to assign to the first car
// OR
// $_POST['auto']: Assign car numbers according to numbering scheme
//
// $_POST['renumber']: if present, renumber all members of the
// group, otherwise only those without a non-zero car number.
//

require_once('inc/car-numbering.inc');
require_once('inc/events.inc');

// $car_numbers is either an integer (the next car number to assign) or an assoc
// mapping partitionid to the next car number to assign in that partition.
function update_number($racerid, $partid) {
  global $update_sql;
  global $car_numbers;

  if (is_int($car_numbers)) {
    $update_sql->execute(array(':carnumber' => $car_numbers,
                               ':racerid' => $racerid));
    ++$car_numbers;
  } else {
    $update_sql->execute(array(':carnumber' => $car_numbers[$partid]++,
                               ':racerid' => $racerid));
  }

  return true;
}

function update_checkin($racerid) {
  global $update_sql;
  global $value;

  if (0 + read_single_value('SELECT COUNT(*) FROM RaceChart'
                               .' WHERE roundid IN (SELECT DISTINCT roundid FROM RaceChart'
                               .'                   WHERE racerid = :racerid)'
                               .' AND (finishtime IS NOT NULL OR finishplace IS NOT NULL)',
                               array(':racerid' => $racerid), 0)) {
    json_failure('alreadyscheduled', "Schedule data already exists for this racer");
    return false;
  }

  if (!$value && !have_permission(REVERT_CHECK_IN_PERMISSION)) {
    not_authorized_body();
    return false;
  }

  $update_sql->execute(array(':value' => $value,
                             ':racerid' => $racerid));
  record_event(EVENT_CHECKIN, array('racerid' => $racerid));
  return true;
}

function update_eligible($racerid) {
  global $update_sql;
  global $value;

  // $value is 1 for eligible, stored as exclude=0
  $update_sql->execute(array(':value' => 1 - $value,
                             ':racerid' => $racerid));
  return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////


$ok = true;
if (!have_permission(EDIT_RACER_PERMISSION)) {
  not_authorized_body();
  $ok = false;
}

if ($ok) {
  if ($_POST['what'] == 'number') {
    if (isset($_POST['auto']) && $_POST['auto']) {
      $car_numbers = base_car_number_by_partition();
    } else {
      $car_numbers = intval($_POST['start']);
    }
    $update_fn = 'update_number';
    $update_sql = $db->prepare('UPDATE RegistrationInfo'
                               .' SET carnumber = :carnumber'
                               .' WHERE racerid = :racerid');
  } else if ($_POST['what'] == 'checkin') {
    $value = isset($_POST['value']) ? $_POST['value'] : 1;
    $update_fn = 'update_checkin';
    $update_sql = $db->prepare('UPDATE RegistrationInfo'
                               .' SET passedinspection = :value'
                               .' WHERE racerid = :racerid');
  } else if ($_POST['what'] == 'eligibility') {
    $value = isset($_POST['value']) ? $_POST['value'] : 1;
    $update_fn = 'update_eligible';
    $update_sql = $db->prepare('UPDATE RegistrationInfo'
                               .' SET exclude = :value'
                               .' WHERE racerid = :racerid');
  } else {
    json_failure('what', "Unrecognized 'what' parameter: ".$_POST['what']);
    $ok = false;
  }
}

if ($ok) {
  $sql = 'SELECT racerid, partitionid FROM RegistrationInfo ';
  $params = array();
  if ($_POST['who'] == 'all') {
    $where = '';
  } else if (substr($_POST['who'], 0, 1) == 'd') {
    $where = 'WHERE partitionid = :partitionid';
    $params[':partitionid'] = substr($_POST['who'], 1);
  } else if (substr($_POST['who'], 0, 1) == 'c') {
    $where = 'WHERE classid = :classid';
    $params[':classid'] = substr($_POST['who'], 1);
  } else if (substr($_POST['who'], 0, 1) == 'r') {
    $where = 'WHERE rankid = :rankid';
    $params[':rankid'] = substr($_POST['who'], 1);
  } else {
    json_failure('who', "Unrecognized 'who' parameter: ".$_POST['who']);
    $ok = false;
  }
}

if ($ok) {
  $stmt = $db->prepare($sql . $where);
  $stmt->execute($params);
  foreach ($stmt as $racer) {
    if ($ok) {
      $ok = call_user_func($update_fn, $racer['racerid'], $racer['partitionid']);
    }
  }
}

if ($ok) {
  json_success();
}

json_out('reload', true);




//////////////////
// if renumber
// For what=number:
// $_POST['start']: number to assign to the first car
// OR
// $_POST['auto']: Assign car numbers according to numbering scheme
//
// $_POST['renumber']: if present, renumber all members of the
// group, otherwise only those without a non-zero car number.
//////////////////////


// If renumbering with mult = 0 then numbering isn't affected by partitionid, and the
// update function could just overwrite the partitionid to 0.
