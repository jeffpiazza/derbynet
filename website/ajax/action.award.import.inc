<?php
require_once('inc/sql-script.inc');
require_once(sql_file_path('column-widths'));
require_once('inc/awards.inc');

start_response();

if (have_permission(SET_UP_PERMISSION)) {

  maybe_populate_award_types();
  
  $fields = array('awardname' => true,
                  'awardtype' => true,
                  'classname' => false,
                  'subgroup' => false,  // rank
                  'sort' => false,
                  // Different ways to identify the recipient, if any
                  'racerid' => false,
                  'carnumber' => false);

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
  if (strlen(trim($_POST['awardname'])) == 0) {
    echo '<failure code="emptyaward">Award name can\'t be empty</failure>';
    $ok = false;
  }
}

if ($ok) {
  $awardtype = $_POST['awardtype'];
  $awardtypeid = read_single_value('SELECT awardtypeid FROM AwardTypes WHERE awardtype = :awardtype',
                                 array(':awardtype' => $awardtype));
  if ($awardtypeid === false) {
    echo '<failure code="unknownawardtype">Unrecognized award type.  Award type must be one of: ';
    $stmt = $db->prepare('SELECT awardtype FROM AwardTypes ORDER BY awardtype');
    $stmt->execute(array());
    $types = array();
    foreach ($stmt as $row) {
      $types[] = $row['awardtype'];
    }
    for ($i = 0; $i < count($types); ++$i) {
      if ($i > 0) { echo ', '; }
      if ($i + 1 == count($types)) { echo 'or '; }
      echo '"'.htmlspecialchars($types[$i], ENT_QUOTES, 'UTF-8').'"';
    }
    echo '.</failure>';
    $ok = false;
  }
}

if ($ok) {
  $classid = 0;
  if (isset($_POST['classname'])) {
    $classname = substr(trim($_POST['classname']), 0, CLASS_COLUMN_WIDTH);
    if (strlen($classname) > 0) {
      $classid = read_single_value('SELECT classid FROM Classes WHERE class = :classname',
                                   array(':classname' => $classname));
      if ($classid === false) {
        echo '<failure code="unknownclass">Unrecognized class</failure>';
        $ok = false;
      }
    }
  }
}

if ($ok) {
  $rankid = 0;
  if (isset($_POST['subgroup'])) {
    $rankname = substr(trim($_POST['subgroup']), 0, RANK_COLUMN_WIDTH);
    if (strlen($rankname) > 0) {
      $rank = read_single_row('SELECT rankid, classid FROM Ranks WHERE rank = :rankname',
                                  array(':rankname' => $rankname));
      if ($rank === false) {
        echo '<failure code="unknownrank">Unrecognized rank</failure>';
        $ok = false;
      } else {
        $rankid = $rank[0];
        if ($classid == 0) {
          $classid = $rank[1];
        } else if ($classid != $rank[1]) {
          echo '<failure code="wrongclass">Subgroup not in this '
          .htmlspecialchars(group_label_lc(), ENT_QUOTES, 'UTF-8')
          .'</failure>';
          $ok = false;
        }
      }
    }
  }
}

if ($ok) {
  if (isset($_POST['sort'])) {
    $sort = 0 + $_POST['sort'];
  } else {
    $sort = read_single_value('SELECT COUNT(*) FROM Awards');
    if ($sort > 0) {
      $sort = read_single_value("SELECT MAX(sort) FROM Awards"
                                ." WHERE awardtypeid <> (SELECT awardtypeid FROM AwardTypes"
                                ."                       WHERE awardtype = 'Ad Hoc')",
                                array());
    }
  }
}

if ($ok) {
  $racerid = 0;
  if (isset($_POST['racerid'])) {
    $racerid = 0 + $_POST['racerid'];
  } else if (isset($_POST['carnumber'])) {
    $nfound = read_single_value('SELECT COUNT(*) FROM RegistrationInfo WHERE carnumber = :carnumber',
                                array(':carnumber' => $_POST['carnumber']));
    if ($nfound == 1) {
      $racerid = read_single_value('SELECT racerid FROM RegistrationInfo WHERE carnumber = :carnumber',
                                   array(':carnumber' => $_POST['carnumber']));
    } else if ($nfound == 0) {
      echo '<failure code="unknowncarnumber">Unknown car number</failure>';
      $ok = false;
    } else {
      echo '<failure code="ambiguouscarnumber">Ambiguous car number</failure>';
      $ok = false;
    }
  }
}

if ($ok) {
  $stmt = $db->prepare('INSERT INTO Awards(awardname, awardtypeid, classid, rankid, racerid, sort)'
                       .' VALUES(:awardname, :awardtypeid, :classid, :rankid, :racerid, :sort)');
  if (!$stmt->execute(array(':awardname' => trim($_POST['awardname']),
                            ':awardtypeid' => $awardtypeid,
                            ':classid' => $classid,
                            ':rankid' => $rankid,
                            ':racerid' => $racerid,
                            ':sort' => $sort))) {
    echo '<failure code="failedinsert">Failed insert.</failure>';
    $ok = false;
  }
}

if ($ok) {
  echo '<success/>';
}

end_response();
?>