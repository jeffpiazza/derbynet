<?php

require_once('inc/newracer.inc');

start_response();

// TODO: Permissions

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
    
if ($ok) {

    $classid = read_single_value('SELECT classid FROM Classes WHERE class = :classname',
                                 array(':classname' => $_POST['classname']));
    if ($classid === false) {
        $stmt = $db->prepare('INSERT INTO Classes(class) VALUES(:classname)');
        $stmt->execute(array(':classname' => $_POST['classname']));
        $classid = read_single_value('SELECT classid FROM Classes WHERE class = :classname',
                                     array(':classname' => $_POST['classname']));
    }

    $rankname = isset($_POST['subgroup']) ? $_POST['subgroup'] : $_POST['classname'];
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

    $carnumber = isset($_POST['carnumber']) ? $_POST['carnumber'] : 0;  // TODO
    // TODO: carname

    insert_new_racer($_POST['firstname'], $_POST['lastname'], $carnumber,
                     $rankid.','.$classid);

    echo '<success/>';
}


end_response();
?>