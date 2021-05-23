<?php
require_once('inc/sql-script.inc');
require_once(sql_file_path('column-widths'));
require_once('inc/awards.inc');

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
      json_failure('missingrequired', "Missing required field $fieldname");
      $ok = false;
      break;
    }
  }
} else {
  json_not_authorized();
  $ok = false;
}

if ($ok) {
  if (strlen(trim($_POST['awardname'])) == 0) {
    json_failure('emptyaward', "Award name can\'t be empty");
    $ok = false;
  }
}

if ($ok) {
  $awardtype = $_POST['awardtype'];
  $awardtypeid = read_single_value('SELECT awardtypeid FROM AwardTypes WHERE awardtype = :awardtype',
                                 array(':awardtype' => $awardtype));
  if ($awardtypeid === false) {
    json_failure('unknownawardtype',
                 "Unrecognized award type.  Award type must be one of: ".
                 $db->query('SELECT awardtype FROM AwardTypes ORDER BY awardtype')
                 .fetchAll(PDO::FETCH_COLUMN).join(', '));
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
        json_failure('unknownclass', "Unrecognized class");
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
        json_failure('unknownrank', "Unrecognized rank");
        $ok = false;
      } else {
        $rankid = $rank[0];
        if ($classid == 0) {
          $classid = $rank[1];
        } else if ($classid != $rank[1]) {
          json_failure('wrongclass', "Subgroup not in this ".group_label_lc());
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
      json_failure('unknowncarnumber', "Unknown car number");
      $ok = false;
    } else {
      json_failure('ambiguouscarnumber', "Ambiguous car number");
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
    json_failure('failedinsert', "Failed insert.");
    $ok = false;
  }
}

if ($ok) {
  json_success();
}
?>