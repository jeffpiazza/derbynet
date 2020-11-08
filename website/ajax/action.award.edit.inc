<?php
/*
 Required: awardid ('new' for new award)
 Optional: 
  name
  awardtypeid
  class_and_rank
  sort
  ballot_depth (0 for "remove from ballot")
*/

require_once('inc/awards-xml.inc');
require_once('inc/schema_version.inc');
require_once('inc/voterid.inc');

if (have_permission(EDIT_AWARDS_PERMISSION)) {
  start_response();
  $awardid = $_POST['awardid'];
  if ($awardid == 'new') {
    $params = array(':awardname' => $_POST['name'],
                    ':awardtypeid' => $_POST['awardtypeid']);
    $fields = '';
    $values = '';
    if (isset($_POST['class_and_rank'])) {
      $class_and_rank = explode(',', $_POST['class_and_rank']);

      $fields .= ', classid';
      $values .= ', :classid';
      $params[':classid'] = $class_and_rank[0];

      if (count($class_and_rank) > 1) {
        $fields .= ', rankid';
        $values .= ', :rankid';
        $params[':rankid'] = $class_and_rank[1];
        // TODO Confirm that rankid and classid agree
      }
    }
    if (isset($_POST['sort'])) {
      $fields .= ', sort';
      $values .= ', :sort';
      $params[':sort'] = $_POST['sort'];
    } else {
      $max_sort = read_single_value("SELECT MAX(sort) FROM Awards"
                                    ." WHERE awardtypeid <> (SELECT awardtypeid FROM AwardTypes"
                                    ."                       WHERE awardtype = 'Ad Hoc')",
                                    array());
      if (!$max_sort) {
        $max_sort = 1;
      }
      $fields .= ', sort';
      $values .= ', :sort';
      $params[':sort'] = 1 + $max_sort;
    }
    $sql = 'INSERT INTO Awards(awardname, awardtypeid'.$fields.')'
        .' VALUES(:awardname, :awardtypeid'.$values.')';
  } else {
    $fields = '';
    $params = array(':awardid' => $awardid);
    if (isset($_POST['name'])) {
      $fields .= ', awardname = :awardname';
      $params[':awardname'] = $_POST['name'];
    }
    if (isset($_POST['awardtypeid'])) {
      $fields .= ', awardtypeid = :awardtypeid';
      $params[':awardtypeid'] = $_POST['awardtypeid'];
    }
    if (isset($_POST['class_and_rank'])) {
      $class_and_rank = explode(',', $_POST['class_and_rank']);
      // classid = 0 means the award is not specific to any class, i.e., it's a
      // pack award.  Similarly, rankid = 0 means the award is not specific to
      // any rank within a class, so it's either a class or pack award.

      $fields .= ', classid = :classid';
      $params[':classid'] = $class_and_rank[0];

      if (count($class_and_rank) > 1) {
        $fields .= ', rankid = :rankid';
        $params[':rankid'] = $class_and_rank[1];
        // TODO Confirm that rankid and classid agree
      }
    }
    if (isset($_POST['sort'])) {
      $fields .= ', sort = :sort';
      $params[':sort'] = $_POST['sort'];
    }
    if (strlen($fields) > 2) {
      $sql = 'UPDATE Awards SET '.substr($fields, 2)
            .' WHERE awardid = :awardid';
    } else {
      $sql = '';
    }
  }

  try {
    if (!empty($sql)) {
      echo '<sql>'.$sql.'</sql>'."\n";
      $stmt = $db->prepare($sql);
      $stmt->execute($params);
    }

    if (isset($_POST['ballot_depth'])) {
      if ($awardid == 'new') {
        $awardid = read_single_value('SELECT awardid FROM Awards'
                                     .' WHERE awardname = :awardname'
                                     .'   AND awardtypeid = :awardtypeid',
                                     array(':awardname' => $_POST['name'],
                                           ':awardtypeid' => $_POST['awardtypeid']));
      }
      set_ballot_depth($awardid, $_POST['ballot_depth']);
    }
    
    echo "<success/>\n";
    emit_award_list(/* include_ad_hoc */ false);
  } catch (Exception $e) {
    echo "<failure code='sql'>".htmlspecialchars($e->getMessage(), ENT_QUOTES, 'UTF-8')."</failure>\n";
  }
  end_response();
} else {
  not_authorized();
}
?>