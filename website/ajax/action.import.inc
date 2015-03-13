<?php

require_once('inc/newracer.inc');
require_once(sql_file_path('column-widths'));

start_response();

if (have_permission(SET_UP_PERMISSION)) {

  $fields = array('lastname' => true,  
                  'firstname' => true,
                  'classname' => true,
                  'carnumber' => false,
                  'carname' => false,
                  'subgroup' => false);

  $ok = true;

  foreach ($fields as $fieldname => $required) {
    if ($required && !isset($_POST[$fieldname])) {
      echo '<failure code="missingrequired">Missing required field '.$fieldname.'</failure>';
      $ok = false;
      break;
    }
  }
} else {
  not_authorized_body();
  $ok = false;
}
    
if ($ok) {

  $classname = substr($_POST['classname'], 0, CLASS_COLUMN_WIDTH);
  // We're failing to find the existing record, but trying to write a
  // new one gives a uniqueness violation against the truncated name.
    $classid = read_single_value('SELECT classid FROM Classes WHERE class = :classname',
                                 array(':classname' => $classname));
    if ($classid === false) {
        $stmt = $db->prepare('INSERT INTO Classes(class) VALUES(:classname)');
        $stmt->execute(array(':classname' => $classname));
        $classid = read_single_value('SELECT classid FROM Classes WHERE class = :classname',
                                     array(':classname' => $classname));

        // Creating a class usually requires creating a corresponding 1st racing round
        $stmt = $db->prepare('INSERT INTO Rounds(round, classid)'
                             .' SELECT 1, classid'
                             .' FROM Classes'
                             .' WHERE classid = :classid'
                             .' AND NOT EXISTS(SELECT 1 FROM Rounds'
                             .'  WHERE Rounds.classid = Classes.classid'
                             .'    AND Rounds.round = 1)');
        $stmt->execute(array(':classid' => $classid));
    }

    $rankname = isset($_POST['subgroup']) ? $_POST['subgroup'] : $classname;
    $rankid = read_single_value('SELECT rankid FROM Ranks WHERE rank = :rankname AND classid = :classid',
                                array(':rankname' => $rankname,
                                      ':classid' => $classid));
    if ($rankid === false) {
        $stmt = $db->prepare('INSERT INTO Ranks(rank, classid) VALUES(:rankname, :classid)');
        $stmt->execute(array(':rankname' => $rankname,
                             ':classid' => $classid));
        $rankid = read_single_value('SELECT rankid FROM Ranks WHERE rank = :rankname AND classid = :classid',
                                    array(':rankname' => $rankname,
                                        ':classid' => $classid));
    }

    $carnumber = isset($_POST['carnumber']) ? $_POST['carnumber'] : 0;
    $carname = isset($_POST['carname']) ? $_POST['carname'] : '';

    insert_new_racer_rankid_and_classid($_POST['firstname'], $_POST['lastname'], $carnumber, $carname,
                                        $rankid, $classid);

    echo '<success/>';
}


end_response();
?>