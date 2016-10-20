<?php
/*
 Required: awardid ('new' for new award)
 Optional: 
  name
  awardtypeid
  class_and_rank
  racerid
  sort  
*/
if (have_permission(EDIT_AWARDS_PERMISSION)) {
  start_response();
  if ($_POST['awardid'] == 'new') {
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
    if (isset($_POST['racerid'])) {
      $fields .= ', racerid';
      $values .= ', :racerid';
      $params[':racerid'] = $_POST['racerid'];
    }
    if (isset($_POST['sort'])) {
      $fields .= ', sort';
      $values .= ', :sort';
      $params[':sort'] = $_POST['sort'];
    } else {
      $max_sort = read_single_value('SELECT MAX(sort) FROM Awards', array());
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
    $params = array(':awardid' => $_POST['awardid']);
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
    if (isset($_POST['racerid'])) {
      $fields .= ', racerid = :racerid';
      $params[':racerid'] = $_POST['racerid'];
    }
    if (isset($_POST['sort'])) {
      $fields .= ', sort = :sort';
      $params[':sort'] = $_POST['sort'];
    }
    $sql = 'UPDATE Awards SET '.substr($fields, 2)
        .' WHERE awardid = :awardid';
  }

  echo '<sql>'.$sql.'</sql>'."\n";
  try {
    $stmt = $db->prepare($sql);
    $stmt->execute($params);
    echo "<success/>\n";
    require('ajax/query.award.list.inc');
  } catch (Exception $e) {
    echo "<failure code='sql'>".htmlspecialchars($e->getMessage(), ENT_QUOTES, 'UTF-8')."</failure>\n";
  }
  end_response();
} else {
  not_authorized();
}
?>