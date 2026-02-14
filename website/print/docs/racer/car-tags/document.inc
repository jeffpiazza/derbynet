<?php
// Generates "car tags", which:
// - are each 2.75 x 8.5" -- just wide enough to be a parking spot for one car
// - are produced four-up per 8.5 x 11" sheet of paper
// - show the racer's name and car number
// - carry a barcode identifying the car
// - optionally show a group-specific image

require_once('inc/path-info.inc');
require_once('inc/data.inc');

class CarTagsDocument extends PrintableRacerDocument {
  function name() { return "Car Tag"; }

  function get_available_options() {
    return array('century_emblem' => array('type' => 'bool',
                                       'desc' => 'Include emblems based on car number',
                                       'default' => true));
  }

  function StartDocument() {
    $this->initialize_pdf('L', 'in', 'Letter');
    $this->initialize_layout(2.75, 8.5);
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
    $this->pdf->RotatedCenteredText($left + 1.1, 2.75,  $racer['carname']);

    $this->pdf->SetFont('Times', '', 15);
    $this->pdf->RotatedCenteredText($left + 1.4, 2.75,  is_null($racer['note']) ? "" : $racer['note']);

    if ($this->get_option('century_emblem')) {
      // 2-inch rank image
      $this->pdf->SetXY($left, 5.5);
      $this->pdf->SetFont('Arial', '', 16);
      $this->pdf->SetFillColor(200, 200, 255);
      $img_path = $this->path_to_century_emblem($racer);
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

  // Group (class) names may vary from one event to another, and the assignment of
  // classid to name is definitely unstable.  This is using the car number
  // instead, and assuming that the hundreds digit identifies which image is
  // desired.
  static protected function path_to_century_emblem(&$racer) {
    // Check if segment-based numbering is enabled
    $numbering_by_segment = read_raceinfo('car-numbering', 0);
    
    if ($numbering_by_segment == 100) {
      // Use car number hundreds place when segment numbering is enabled
      $grade = intval($racer['carnumber'] / 100);
      return image_file_path("centuries".DIRECTORY_SEPARATOR. $grade . '00-series');
    } else {
      // Use rank/class name to determine the emblem when not using segment numbering
      $rank_name = strtolower($racer['rank']);
      
      // Map rank names to century series (adjust these mappings as needed)
      $rank_to_century = array(
        'lion' => 0,
        'tiger' => 1,
        'wolf' => 2,
        'bear' => 3,
        'webelos' => 4,
        'arrow of light' => 5,
        'aol' => 5
      );
      
      foreach ($rank_to_century as $key => $century) {
        if (strpos($rank_name, $key) !== false) {
          return image_file_path("centuries".DIRECTORY_SEPARATOR. $century . '00-series');
        }
      }
      
      // Fallback to car number method if rank name doesn't match
      $grade = intval($racer['carnumber'] / 100);
      return image_file_path("centuries".DIRECTORY_SEPARATOR. $grade . '00-series');
    }
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
