<?php

require_once('inc/partitions.inc');
require_once('inc/path-info.inc');
require_once('inc/photo-config.inc');

class DriversLicenseBackDocument extends PrintableRacerDocument {
  function name() { return "Driver's License Back"; }

  function get_available_options() {
    return array();
  }

  function StartDocument() {
    $this->initialize_pdf('L', 'in', 'Letter');
    $this->initialize_layout(3.5, 2.5);
    $this->layout->SetHorizontalGap(0.1);
    $this->layout->SetVerticalGap(0.1);
  }

  function DrawOneAt($x, $y, &$racer) {   
    $this->pdf->RoundedRect(11.0-$x-3.5, $y, 3.5, 2.5, 0.125);
    $this->DrawBanner($x, $y);

    if ($racer['carphoto']) {
      $this->pdf->Image($racer['carphoto'], 11.0-$x-3.5 + 0.5, $y + 0.5, 2.5);
    } else {
      $this->pdf->SetFont('Arial', '', 10);
      $this->pdf->SetXY(11.0-$x-3.5 + 0.5, $y + 0.5);
      $this->pdf->SetFillColor(0x80);
      $this->pdf->Cell(2.5, 1.75, "No car photo", /* frame */ 1,
                       /* ln */ 0, /* align */ 'C', /* fill */ true);
    }
    $this->DrawBannerText($x, $y, $racer);
  }

  // The "banner" comprises a blue horizontal line, with small lines above and below.
  protected function DrawBanner($x, $y) {
    $this->pdf->SetFillColor(0x02, 0x38, 0x82);

    $this->pdf->Rect(11.0-$x-3.5 + 0.125, $y + 0.125, 3.25, 0.010, 'F');
    $this->pdf->Rect(11.0-$x-3.5 + 0.125, $y + 0.150, 3.25, 0.010, 'F');

    $this->pdf->Rect(11.0-$x-3.5 + 0.125, $y + 0.175, 3.25, 0.180, 'F');

    $this->pdf->Rect(11.0-$x-3.5 + 0.125, $y + 0.375, 3.25, 0.010, 'F');
    $this->pdf->Rect(11.0-$x-3.5 + 0.125, $y + 0.400, 3.25, 0.010, 'F');
  }

  protected function DrawBannerText($x, $y, $racer) {
    $this->pdf->SetFont('Arial', 'B', 10);
    $this->pdf->SetTextColor(0xf7, 0xd1, 0x17);
    $this->pdf->Text(11.0-$x-3.5 + 0.175, $y + 0.305, strval($racer['carname']));	
    $this->pdf->SetTextColor(0);
  }
}
