<?php
// Adding a class:
// $_POST['name']

require_once('inc/schema_version.inc');
require_once(sql_file_path('column-widths'));

start_response();

if (have_permission(SET_UP_PERMISSION)) {
  if (isset($_POST['name'])) {
    $classname = substr(trim($_POST['name']), 0, CLASS_COLUMN_WIDTH);
    if ($classname == '') {
      echo "<failure code='noname'>No name provided for the new ".group_label()."</failure>\n";
    } else {
      $stmt = $db->prepare('INSERT INTO Classes(class) VALUES(:name)');
      $stmt->execute(array(':name' => $classname));

      $classid = read_single_value('SELECT classid FROM Classes WHERE class = :classname',
                                   array(':classname' => $classname));
      if (schema_version() >= 2) {
        $nclasses = read_single_value('SELECT COUNT(*) FROM Classes');
        $stmt = $db->prepare('UPDATE Classes'
                             .' SET sortorder = :nclasses'
                             .' WHERE classid = :classid');
        $stmt->execute(array(':classid' => $classid,
                             ':nclasses' => $nclasses));
      }

      // Creating a class usually requires creating a corresponding 1st racing round
      $stmt = $db->prepare('INSERT INTO Rounds(round, classid)'
                           .' SELECT 1, classid'
                           .' FROM Classes'
                           .' WHERE classid = :classid'
                           .' AND NOT EXISTS(SELECT 1 FROM Rounds'
                           .'  WHERE Rounds.classid = Classes.classid'
                           .'    AND Rounds.round = 1)');

      $stmt->execute(array(':classid' => $classid));

      // Manufacture a "generic" rank name for this class.  It isn't really
      // well-defined to be creating a class without a rank if we're actually
      // using rank names.
      $stmt = $db->prepare('INSERT INTO Ranks(rank, classid) VALUES(:rankname, :classid)');
      $stmt->execute(array(':rankname' => $classname,
                           ':classid' => $classid));

      if (schema_version() >= 2) {
        $nranks = read_single_value('SELECT COUNT(*) FROM Ranks WHERE classid = :classid',
                                    array(':classid' => $classid));
        $stmt = $db->prepare('UPDATE Ranks'
                             .' SET sortorder = :nranks'
                             .' WHERE rankid = :rankid');
        $stmt->execute(array(':nranks' => $nranks,
                             ':rankid' => $rankid));
      }

      echo "<success/>\n";
    }
  } else {
    echo "<failure code='missing-args'>Unrecognized argument signature</failure>\n";
  }
} else {
  echo "<failure code='permission'>Not authorized</failure>\n";
}

end_response();
?>