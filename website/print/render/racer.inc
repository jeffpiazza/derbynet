<?php
require_once('inc/car-numbering.inc');
require_once('inc/data.inc');
require_once('inc/partitions.inc');
require_once('inc/schema_version.inc');
require_once('inc/action-helpers.inc');
require_once('print/inc/printable_racer_document.inc');
require_once('fpdf/ext/combined.php');

// $_GET['options']
// $_GET['car_nos']
// $_GET['walkins'] ("p" suffix for per-partition)
//
// $_GET['ids']
// $_GET['where']

function derez_photos(&$racer) {
  $racer['imagefile_fullrez'] = $racer['imagefile'];
  // If there's a 400x400 version of the image, use that, but otherwise use
  // the full (cropped) photo, don't try to create a lower-rez one.
  $imgfile = headshots()->lookup_or_any('400x400')->file_path(basename($racer['imagefile']));
  if (is_file($imgfile)) {
    $racer['imagefile'] = $imgfile;
  }

  $racer['carphoto_fullrez'] = $racer['carphoto'];
  $imgfile = car_photo_repository()->lookup_or_any('400x400')->file_path(basename($racer['carphoto']));
  if (is_file($imgfile)) {
    $racer['carphoto'] = $imgfile;
  }
}

function emit_walkins($next, $howmany) {
  global $doc;
  for ($car_no = $next; $car_no < $next + $howmany; ++$car_no) {
    $racer = array('racerid' => 0, 'carnumber' => $car_no,
                   'lastname' => '', 'firstname' => '',
                   'carname' => '', 'classid' => 0, 'class' => '', 'rankid' => 0, 'rank' => '',
                   'imagefile' => '', 'carphoto' => '',
                   'note' => '', 'barcode' => sprintf('PWDwi%03d.', $car_no));
    $doc->DrawOne($racer);
  }
}

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
  // customized to individual racers.  (This feature isn't accessible by any
  // current code path.)
  if (isset($_GET['car_nos'])) {
    $sql = $base_sql.' WHERE carnumber = :carnumber';

    function merge_and_draw_one($car_no) {
      global $db, $sql, $doc;
      $racer = array('racerid' => 0, 'carnumber' => $car_no, 'lastname' => '', 'firstname' => '',
                     'carname' => '', 'classid' => 0, 'class' => '', 'rankid' => 0, 'rank' => '',
                     'imagefile' => '', 'carphoto' => '',
                     'barcode' => sprintf('PWD%03d.', $car_no));
      $rs = $db->prepare($sql);
      $rs->execute(array(':carnumber' => $car_no));
      $row = $rs->fetch(PDO::FETCH_ASSOC);
      if ($row) {
        convert_strings($row);
        clean_fake_photos($row);
        derez_photos($row);
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
      derez_photos($racer);
      $racer['barcode'] = sprintf('PWDid%03d.', $racer['racerid']);
      $doc->DrawOne($racer);
    }

    if (isset($_GET['ids'])) {
      if (!empty($_GET['ids'])) {
        $sql = $base_sql.' WHERE racerid = :racerid';
        $stmt = $db->prepare($sql);
        foreach (explode(',', $_GET['ids']) as $racerid) {
          $stmt->execute(array(':racerid' => $racerid));
          $racer = $stmt->fetch(PDO::FETCH_ASSOC);
          draw_one_racer($racer);
        }
      }
    } else {
      $sql = $base_sql;
      // if (isset($_GET['where'])) {
      //   $sql = $sql.' WHERE '.$_GET['where'];
      // }
      $sql = $sql.' ORDER BY lastname, firstname, carnumber';

      foreach ($db->query($sql) as $racer) {
        draw_one_racer($racer);
      }
    }
  }

  if (isset($_GET['walkins'])) {
    $walkins = $_GET['walkins'];
    if (str_ends_with($walkins, 'p')) {  // "per partition"
      // if $mult is zero, then next_car_numbers will just repeat the same car
      // numbers for each partition.
      $prev = 0;
      $walkins = intval(substr($walkins, 0, -1));
      foreach (next_car_numbers() as $partition) {
        $next = max($prev + $walkins, $partition['next_carnumber']);
        emit_walkins($next, $walkins);
        $prev = $next;
      }
    } else {
      list($mult, $smallest) = read_car_numbering_values();
      $max_carno = read_single_value('SELECT COALESCE(MAX(carnumber), 0) FROM RegistrationInfo');
      if ($mult == 0) {
        $next = 1 + $max_carno;
      } else {
        // Assumes that the effect of $smallest is already reflected in MAX(carnumber)
        $next = 1 + $mult * (1 + intdiv($max_carno, $mult));
      }
      emit_walkins($next, intval($walkins));
    }
  }

  $doc->Output();
}
?>
