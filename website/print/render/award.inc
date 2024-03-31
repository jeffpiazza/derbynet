<?php

require_once('inc/data.inc');
require_once('inc/schema_version.inc');
require_once('inc/action-helpers.inc');
require_once('print/inc/printable_award_document.inc');
require_once('fpdf/ext/combined.php');


if (!is_subclass_of(document_class(), 'PrintableAwardDocument')) {
  echo "<p>Not ".document_class()."</p>\n";
  foreach (get_declared_classes() as $c) {
    if (is_subclass_of($c, 'PrintableAwardDocument') && !(new ReflectionClass($c))->isAbstract()) {
      echo "<p>Loaded class ".$c."</p>\n";
    }
  }
} else {

  $doc = new_document();

  $doc->StartDocument();

  if (isset($_GET['options'])) {
    $doc->set_options(json_decode($_GET['options'], true));
  }

  $base_sql = 'SELECT awardid, awardname, awardtype, classid, rankid, racerid '
  .' FROM '.inner_join('Awards', 'AwardTypes',
                       'Awards.awardtypeid = AwardTypes.awardtypeid');

  $sql = $base_sql;
  if (($_GET['ids'])) {
    $sql = $sql.' WHERE awardid IN ('.$_GET['ids'].',0)';
  // } else if (isset($_GET['where'])) {
  // $sql = $sql.' WHERE '.$_GET['where'];
  }

  foreach ($db->query($sql) as $award) {
    $racer = read_single_row(
      'SELECT carnumber, lastname, firstname, carname, '
      .' class, rank, imagefile,'
      .(schema_version() < 2 ? ' \'\' as carphoto' : ' carphoto')
      .' FROM '.inner_join('RegistrationInfo', 'Classes',
                           'RegistrationInfo.classid = Classes.classid',
                           'Ranks',
                           'RegistrationInfo.rankid = Ranks.rankid')
      .' WHERE racerid = :racerid',
      array(':racerid' => $award['racerid']), PDO::FETCH_ASSOC);
    if ($racer) {
      $award['carnumber'] = $racer['carnumber'];
      $award['lastname'] = $racer['lastname'];
      $award['firstname'] = $racer['firstname'];
      $award['carname'] = $racer['carname'];
      $award['class'] = $racer['class'];
      $award['rank'] = $racer['rank'];
      $award['imagefile'] = $racer['imagefile'];
      $award['carphoto'] = $racer['carphoto'];
    }
    
    convert_strings($award);
    
    $doc->DrawOne($award);
  }

  $doc->Output();
}

?>
