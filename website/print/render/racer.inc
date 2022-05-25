<?php
require_once('inc/data.inc');
require_once('inc/partitions.inc');
require_once('inc/schema_version.inc');
require_once('inc/action-helpers.inc');
require_once('print/inc/printable_racer_document.inc');
require_once('fpdf/ext/combined.php');

// $_GET['options']
// $_GET['car_nos']
//
// $_GET['ids']
// $_GET['where']

if (!is_subclass_of(document_class(), 'PrintableRacerDocument')) {
  echo "<p>Not ".document_class()."</p>\n";
  /*
    Present a form that lets the user pick groups of racers, or individual
    racers, or groups of awards, or individual awards.  ("Most recently checked
    in" would be desirable, too.)
  */
  foreach (get_declared_classes() as $c) {
    if (is_subclass_of($c, 'PrintableRacerDocument') && !(new ReflectionClass($c))->isAbstract()) {
      echo "<p>Loaded class ".$c."</p>\n";
    }
  }
} else {
  $doc = new_document();

  if (isset($_GET['options'])) {
    $doc->set_options(json_decode($_GET['options'], true));
  }

  $doc->StartDocument();

  $base_sql = 'SELECT racerid, carnumber, lastname, firstname, carname, '
    .' RegistrationInfo.classid, class, RegistrationInfo.rankid, rank,'
    .' imagefile,'
    .(schema_version() < 2 ? ' \'\' as carphoto' : ' carphoto').', '
    .(schema_version() < PARTITION_SCHEMA ? ' \'\' as note' : ' note')
    .' FROM '.inner_join('RegistrationInfo', 'Classes',
                         'RegistrationInfo.classid = Classes.classid',
                         'Ranks',
                         'RegistrationInfo.rankid = Ranks.rankid');

  // Specifying by car numbers means we're making generic printables, not
  // customized to individual racers.
  if (isset($_GET['car_nos'])) {
    $sql = $base_sql.' WHERE carnumber = :carnumber';

    function merge_and_draw_one($car_no) {
      global $db, $sql, $doc;
      $racer = array('racerid' => 0, 'carnumber' => $car_no, 'lastname' => '', 'firstname' => '',
                     'carname' => '', 'classid' => 0, 'class' => '', 'rankid' => 0, 'rank' => '',
                     'imagefile' => '', 'carphoto' => '',
                     'barcode' => 'PWD'.sprintf('%03d', $car_no));
      $rs = $db->prepare($sql);
      $rs->execute(array(':carnumber' => $car_no));
      $row = $rs->fetch(PDO::FETCH_ASSOC);
      if ($row) {
        convert_strings($row);
        clean_fake_photos($row);
        foreach ($row as $key => $val) {
          $racer[$key] = $val;
        }
      }

      $doc->DrawOne($racer);
    }

    foreach (explode(',', $_GET['car_nos']) as $car_number_range) {
      if (is_numeric($car_number_range)) {
        merge_and_draw_one(0 + $car_number_range);
      } else {
        $bounds = explode('-', $car_number_range, 2);
        $upper = 0 + $bounds[1];
        for ($car_no = 0 + $bounds[0]; $car_no <= $upper; ++$car_no) {
          merge_and_draw_one($car_no);
        }
      }
    }
  } else {
    function draw_one_racer(&$racer) {
      global $doc;
      convert_strings($racer);
      clean_fake_photos($racer);
      $racer['barcode'] = 'PWDid'.sprintf('%03d', $racer['racerid']);
      $doc->DrawOne($racer);
    }

    if (isset($_GET['ids'])) {
      $sql = $base_sql.' WHERE racerid = :racerid';
      $stmt = $db->prepare($sql);
      foreach (explode(',', $_GET['ids']) as $racerid) {
        $stmt->execute(array(':racerid' => $racerid));
        $racer = $stmt->fetch(PDO::FETCH_ASSOC);
        draw_one_racer($racer);
      }
    } else {
      $sql = $base_sql;
      if (isset($_GET['where'])) {
        $sql = $sql.' WHERE '.$_GET['where'];
      }
      $sql = $sql.' ORDER BY lastname, firstname, carnumber';

      foreach ($db->query($sql) as $racer) {
        draw_one_racer($racer);
      }
    }
  }

  $doc->Output();
}
?>