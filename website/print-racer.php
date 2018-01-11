<?php

require_once('inc/data.inc');
require_once('inc/schema_version.inc');
require_once('inc/action-helpers.inc');
require_once('inc/racer_document_classes.inc');
require_once('fpdf/ext/combined.php');

// $_GET['document']
// TODO $_GET['where']
// TODO $_GET['order-by']
// $_GET['options']

if (!is_subclass_of($_GET['document'], 'PrintableRacerDocument')) {
  start_response();
  echo "<failure code='unknown-document-class'>No such document class</failure>\n";
  end_response();
} else {

  $doc = new $_GET['document'];

  $doc->StartDocument();

  if (isset($_GET['options'])) {
    $doc->set_options(unserialize($_GET['options']));
  }

  $sql = 'SELECT racerid, carnumber, lastname, firstname, carname, '
  .' RegistrationInfo.classid, class, RegistrationInfo.rankid, rank,'
  .' imagefile,'
  .(schema_version() < 2 ? ' \'\' as carphoto' : ' carphoto')
  .' FROM '.inner_join('RegistrationInfo', 'Classes',
                       'RegistrationInfo.classid = Classes.classid',
                       'Ranks',
                       'RegistrationInfo.rankid = Ranks.rankid')
  .' ORDER BY lastname, firstname, carnumber';

  foreach ($db->query($sql) as $racer) {
    foreach ($racer as $key => $value) {
      if (is_string($value)) {
        $racer[$key] = iconv('UTF-8', 'windows-1252', $value);
      }
    }

    $racer['barcode'] = 'PWDid'.sprintf('%03d', $racer['racerid']);

    $doc->DrawOne($racer);
  }

  $doc->Output();
}

?>