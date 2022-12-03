<?php

require_once('inc/path-info.inc');
require_once('inc/photo-config.inc');

class CheckInCardDocument extends PrintableRacerDocument {
  function name() { return "Check-In Card"; }

  function get_available_options() {
    return array();
  }

  function StartDocument() {
    $this->initialize_pdf('P', 'in', 'Letter');
    // 3 3⁄8 in × 2 1⁄8 in
    $this->initialize_layout(3.375, 2.025);
    $this->layout->SetHorizontalGap(0.05);
    $this->layout->SetVerticalGap(0.05);
  }

  function DrawOneAt($x, $y, &$racer) {
    $this->pdf->Rect($x, $y, 3.375, 2.025, 'D');

    $this->pdf->Image(image_file_path('emblem'),
                      $x + 0.05, $y + 0.05, 0.75, 0.75);

    $name = array(new SetFontTag('Times', '', 18), $racer['firstname']." ",
                  new SetFontTag('Times', 'B', 18), $racer['lastname']);
    $w = $this->pdf->SequenceWidth($name) + 0.1;
    $this->pdf->SetFillColor(255);  // white
    $this->pdf->SetAlpha(0.85);
    $this->pdf->Rect($x + (3.375 - $w) / 2, $y + 0.55 - 18/72, $w, 18 / 72 * 1.5, 'F');
    $this->pdf->SetAlpha(1.0);
    $this->pdf->SetFillColor(0);  // black

    $this->pdf->CenteredSequence($x + 3.375 / 2, $y + 0.55, $name);

    $this->pdf->CenteredSequence($x + 3.375 / 2, $y + 1.05,
                                 array(new SetFontTag('Times', 'B', 24),
                                       strval($racer['carnumber'])));

    $this->pdf->HorizontalBarcode($x + 0.6875, $y + 1.50, 2.0, 0.20, $racer['barcode']);
  }
}

?>
