<?php

require_once('inc/partitions.inc');
require_once('inc/path-info.inc');
require_once('inc/photo-config.inc');

class DriversLicensePPDocument extends PrintableRacerDocument {
  function name() { return "Driver's License - Powerpuff Derby"; }

  function get_available_options() {
    return array(
      'use_photo' => array('type' => 'bool',
                           'desc' => 'Include racer\'s photo',
                           'default' => true),
      'use_barcode' => array('type' => 'bool',
                             'desc' => 'Include barcode',
                             'default' => true),
      'date_of_issue' => array('type' => 'string',
                               'desc' => 'Date of Issue',
                               'default' => date('F j, Y')),
      );
  }

  function StartDocument() {
    $this->initialize_pdf('L', 'in', 'Letter');
    $this->initialize_layout(3.5, 2.5);
    $this->layout->SetHorizontalGap(0.1);
    $this->layout->SetVerticalGap(0.1);
  }

  function DrawOneAt($x, $y, &$racer) {
    $issue_date = new DateTimeImmutable($this->get_option('date_of_issue'));
    
    $this->pdf->RoundedRect($x, $y, 3.5, 2.5, 0.125);

    $this->pdf->Image(dirname(__FILE__).'/PP-LIC-02.png', $x, $y, 3.5);
    
    $this->pdf->DrawSequence($x + 0.25, $y + 0.8,
                             array(new SetFontTag('Times', '', 14), $racer['firstname']." ",
                                   new SetFontTag('Times', 'B', 14), $racer['lastname']));

    if (use_groups()) {
      $this->pdf->DrawSequence($x + 0.25, $y + 1.0,
                               array(new SetFontTag('Times', '', 10), group_label().": ",
                                     new SetFontTag('Times', 'B', 10), $racer['class']));
    }

    if (!$this->get_option('use_photo')) {
    } else if ($racer['imagefile']) {
      $this->pdf->Image($racer['imagefile'], $x + 2.35, $y + 0.5, 1.00);
    } else {
      $this->pdf->SetFont('Arial', '', 10);
      $this->pdf->SetXY($x + 2.35, $y + 0.5);
      $this->pdf->SetFillColor(0x80);
      $this->pdf->Cell(1.00, 1.33, "No photo", /* frame */ 1,
                       /* ln */ 0, /* align */ 'C', /* fill */ true);
    }

    $this->pdf->CenteredSequence($x + 1.75, $y + 1.5,
                                 array(new SetFontTag('Arial', 'B', 24), strval($racer['carnumber'])));

    $this->pdf->SetFillColor(0);
    if ($this->get_option('use_barcode')) {
      $this->pdf->HorizontalBarcode($x + 1.20, $y + 1.90, 2.0, 0.20, $racer['barcode']);
    }
    
//    $this->pdf->DrawSequence($x + 0.75, $y + 2.25,
//                             array(new SetFontTag('Times', '', 10), 'Restrictions: ',
//                                   new SetFontTag('Times', 'B', 10), 'Must follow Cub Scout Law'));
    if (false) {
      $this->pdf->DrawSequence($x + 0.75, $y + 2.10,
                               array(new SetFontTag('Times', '', 10), 'Date of Issue: ',
                                     new SetFontTag('Times', 'B', 10), $issue_date->format('l, F j, Y')));
    }
  }

  // The "banner" comprises a blue horizontal line, with small lines above and below.
  protected function DrawBanner($x, $y) {
    $this->pdf->SetFillColor(0x02, 0x38, 0x82);

    $this->pdf->Rect($x + 0.125, $y + 0.125, 3.25, 0.010, 'F');
    $this->pdf->Rect($x + 0.125, $y + 0.150, 3.25, 0.010, 'F');

    $this->pdf->Rect($x + 0.125, $y + 0.175, 3.25, 0.180, 'F');

    $this->pdf->Rect($x + 0.125, $y + 0.375, 3.25, 0.010, 'F');
    $this->pdf->Rect($x + 0.125, $y + 0.400, 3.25, 0.010, 'F');
  }

  protected function DrawBannerText($x, $y) {
    $this->pdf->SetFont('Arial', 'B', 10);
    $this->pdf->SetTextColor(0xf7, 0xd1, 0x17);
    $this->pdf->Text($x + 0.175, $y + 0.305, 'Pinewood Derby Racing License');
    $this->pdf->SetTextColor(0);
  }
}
