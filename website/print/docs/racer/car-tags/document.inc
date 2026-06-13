<?php
// Generates "car tags", which:
// - are each 2.75 x 8.5" -- just wide enough to be a parking spot for one car
// - are produced four-up per 8.5 x 11" sheet of paper
// - show the racer's name and car number
// - carry a barcode identifying the car
// - optionally show a group-specific image

require_once('inc/path-info.inc');
require_once('inc/emblems.inc');

function default_century_image_assignments() {
  $all_emblem_images = all_emblem_image_stems();
  $assignments = array();
  $max_carno = read_single_value('SELECT COALESCE(MAX(carnumber), 999) FROM RegistrationInfo');
  $max_century = intdiv($max_carno, 100);
  for ($century = 0; $century <= $max_century; ++$century) {
    if ($century < count($all_emblem_images)) {
      $assignments[] = $all_emblem_images[$century];
    } else {
      $assignments[] = null;
    }
  }
  return $assignments;
}

function default_partition_image_assignments() {
  $all_emblem_images = all_emblem_image_stems();
  $assignments = array();
  $n_parts = read_single_value('SELECT COUNT(*) FROM Partitions');
  for ($p = 0; $p < $n_parts; ++$p) {
    if ($p < count($all_emblem_images)) {
      $assignments[] = $all_emblem_images[$p];
    } else {
      $assignments[] = null;
    }
  }
  return $assignments;
}

class CarTagsDocument extends PrintableRacerDocument {
  protected $partitions = array();  // gives partition sortorder indexed by partitionid

  function name() { return "Car Tag"; }

  function get_available_options() {
    // The 'default' value here gives default assignments by car number and by
    // partition sortorder.
    //
    // The option value that's received is different: an array whose first
    // element is a string, and the remaining elements give emblem numbers,
    // indexed either by car number century or partition sortorder.
    return array('emblems' => array('type' => 'emblems',
                                    'desc' => 'Include emblems',
                                    'default' =>
                                        array('style' => 'century',
                                              'centuries' => default_century_image_assignments(),
                                              'partitions' => default_partition_image_assignments())));
  }

  function StartDocument() {
    global $db;
    $this->initialize_pdf('L', 'in', 'Letter');
    $this->initialize_layout(2.75, 8.5);

    $nparts = read_single_value('SELECT 1 + COALESCE(MAX(partitionid), 1) FROM Partitions', array(), 1);
    $this->partitions = array_fill(0, $nparts, 0);
    foreach($db->query('SELECT partitionid, sortorder FROM Partitions ORDER BY partitionid') AS $row) {
      $this->partitions[$row['partitionid']] = $row['sortorder'];
    }
  }

  function DrawOneAt($left, $top, &$racer) {
    // This Rect would draw dividing lines between the tags, but it's difficult
    // to cut precisely on the line for every page in a stack, so better not to
    // highlight the problem by showing the lines.
    //
    // $this->pdf->Rect($left, 0,  2.75, 8.5);

    $this->pdf->SetFillColor(0,0,0);
    // Small barcode along the top, sometimes more reliable to scan
    $this->pdf->HorizontalBarcode($left + 0.625, 0.40, 1.5, 0.25, $racer['barcode']);
    // Large barcode
    $this->pdf->VerticalBarcode($left + 1.75, 0.75, 0.5, 4, $racer['barcode']);

    $this->RotatedCenteredName($left + 0.7, 2.75, $racer);

    $this->pdf->SetFont('Times', 'I', 15);
    $this->pdf->RotatedCenteredText($left + 1.1, 2.75, $racer['carname']);

    $this->pdf->SetFont('Times', '', 15);
    $this->pdf->RotatedCenteredText($left + 1.4, 2.75,  is_null($racer['note']) ? "" : $racer['note']);

    $emblems = $this->get_option('emblems');
    if ($emblems[0] != 'none') {
      // 2-inch rank image
      $this->pdf->SetXY($left, 5.5);
      $this->pdf->SetFont('Arial', '', 16);
      $this->pdf->SetFillColor(200, 200, 255);

      $img_path = path_to_emblem_image(
        $emblems[($emblems[0] == 'by-carno')
                 ? 1+ intval($racer['carnumber'] / 100)
                 : $this->partitions[intval($racer['partitionid'])]]);
      // $this->pdf->Rect($left, 5.5, 2.75, 2.00);
      if ($img_path && is_readable($img_path)) {
        list($width, $height) = @getimagesize($img_path);
        if ($height != 0) {
          if ($width < $height) {
            $img_height = 2.00;
          } else {
            $img_height = $height/$width * 2.00;
          }
          $dx = (2.75 - ($width/$height)*$img_height) / 2;
          $this->pdf->Image($img_path, $left + $dx, 7.5 - $img_height,
                            /* calculate width */0, $img_height);
        }
      }
    }

    // 1/2-inch letters at the bottom of an 8.5-inch tag.
    // We want to get as close to the edge as we can print.
    $this->pdf->SetFont('Times', 'B', 36);
    $this->pdf->SetXY($left, 7.75);
    $this->pdf->SetFillColor(255, 255, 200);
    $this->pdf->Cell(2.75, 0.5, $racer['carnumber'], /* border */ 0, /* ln */ 0, "C");
  }

  protected function RotatedCenteredName($cx, $cy, &$racer) {
    $this->pdf->StartTransform();
    $this->pdf->Rotate(90, $cx, $cy);
    $this->pdf->CenteredSequence($cx, $cy,
                                 array(new SetFontTag('Times', '', 30), $racer['firstname']." ",
                                       new SetFontTag('Times', 'B', 30), $racer['lastname']));
    $this->pdf->StopTransform();
  }
}
?>
