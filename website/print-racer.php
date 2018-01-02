<?php

require_once('inc/data.inc');
require_once('inc/schema_version.inc');
require_once('inc/action-helpers.inc');
require_once('fpdf/ext/combined.php');

// $_GET['template']
// TODO $_GET['where']
// TODO $_GET['order-by']

abstract class PrintableRacerDocument {
  protected $pdf;

  function __construct() {
    $this->pdf = new PDF_Combined('L', 'in', 'Letter');
  }
  
  abstract public function DrawOne(&$racer);

  public function Output() {
    $this->pdf->Output();
  }

}

if (!@include('printables/racer-' . $_GET['template'] . '.inc')) {
  start_response();
  echo "<failure code='unknown-template'>No such template</failure>\n";
  end_response();
} else {
  
  $doc = make_new_document();

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
    $doc->DrawOne($racer);
  }

  $doc->Output();
}

?>