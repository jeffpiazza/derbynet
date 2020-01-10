<?php

require_once('inc/photo-config.inc');

class DriversLicenseDocument extends PrintableRacerDocument {
  function name() { return "Driver's License"; }

  function get_available_options() {
    return array(
      'use_photo' => array('type' => 'bool',
                           'desc' => 'Include racer\'s photo',
                           'default' => true),
      'use_barcode' => array('type' => 'bool',
                             'desc' => 'Include barcode',
                             'default' => true),
      );
  }

  function StartDocument() {
    //$this->initialize_pdf('P', 'in', array(4,6));
    $this->initialize_pdf('L', 'in', 'Letter');
    $this->initialize_layout(3.0, 1.75);
    $this->layout->SetHorizontalGap(0.1);
    $this->layout->SetVerticalGap(0.1);
  }

  function DrawOneAt($x, $y, &$racer) {
    $this->DrawBanner($x, $y);
    $this->pdf->RoundedRect($x, $y, 3, 1.75, 0.1);

    // Washout the background for better visibility
    $this->pdf->SetAlpha(0.15);
    $this->pdf->Image(dirname(__FILE__).'/awana-clubs-logo-color.jpg', $x + 0.90, $y + 0.60, 2);
    $this->pdf->SetAlpha(1);

    //$this->DrawBanner($x, $y);
    // We write the banner text at the end, over the top of the photo
    // $this->DrawBannerText($x, $y);

    // Logo in the lower left
    //$this->pdf->Image(dirname(__FILE__).'/cubscout_emblem_300.png',
    //                  $x + 0.05, $y + 1.95, 0.5, 0.5);

    $this->pdf->DrawSequence($x + 0.90, $y + 0.75,
                             array(new SetFontTag('Times', 'B', 14), $racer['firstname']." ",
                                   new SetFontTag('Times', 'B', 14), $racer['lastname']));

    $this->pdf->DrawSequence($x + 0.90, $y + 0.95,
                             array(new SetFontTag('Times', '', 10), group_label().": ",
                                   new SetFontTag('Times', 'B', 10), $racer['class']));

    if (!$this->get_option('use_photo')) {
    } else if ($racer['imagefile']) {

      //$this->pdf->CenteredSequence($x + 0.1, $y + 1.075,
      //                           array(new SetFontTag('Arial', '', 8), $racer['imagefile']));
      //$this->pdf->Image($racer['imagefile'], $x + , $y + 0.25, 1.00);
      $this->pdf->Image($racer['imagefile'], $x + 0.1, $y + 0.6, 0.75);
      //$this->pdf->Image(dirname(__FILE__).'/yellow_racing_car.png', $x + 0.1, $y + 0.6, 0.75);
    } else {
      $this->pdf->Image(dirname(__FILE__).'/yellow_racing_car.png', $x + 0.1, $y + 0.75, 0.75);
//      $this->pdf->SetFont('Arial', '', 10);
//      $this->pdf->SetXY($x + 0.1, $y + 0.6);
//      $this->pdf->SetFillColor(0x80);
//      $this->pdf->Cell(0.75, 1.00, "No photo", /* frame */ 1,
//                       /* ln */ 0, /* align */ 'C', /* fill */ true);
    }

    $this->pdf->CenteredSequence($x + 2.35, $y + 1.075,
                                 array(new SetFontTag('Arial', 'B', 24), $racer['carnumber']));

    $this->pdf->SetFillColor(0);
    if ($this->get_option('use_barcode')) {
      $this->pdf->HorizontalBarcode($x + 0.90, $y + 1.15, 2.0, 0.20, $racer['barcode']);
    }
//    $this->pdf->SetFont('Arial', 'B', 8);
//    $this->pdf->Text($x + 0.90, $y + 1.15, $racer['barcode']);

    $this->pdf->DrawSequence($x + 2.80, $y + 0.60,
                               array(new SetFontTag('Times', 'B', 8), str_replace("PWDid", "", $racer['barcode'])));
    
//    $this->pdf->DrawSequence($x + 0.75, $y + 2.25,
//                             array(new SetFontTag('Times', '', 10), 'Restrictions: ',
//                                   new SetFontTag('Times', 'B', 10), 'Must follow Cub Scout Law'));
    $this->pdf->DrawSequence($x + 0.90, $y + 1.5,
                             array(new SetFontTag('Times', '', 10), 'Issued on the ',
                                   new SetFontTag('Times', '', 10), $this->ordinal(date('z')), ' day of ', date('Y')));

    $this->pdf->SetFillColor(0x02, 0x38, 0x82);
    $this->pdf->Rect($x + 0.5, $y + 1.535, 2.5, 0.15, 'F');
    $this->pdf->LinearGradient($x + 0.5, $y + 1.535, 1.5, 0.15,
                               array(129, 155, 192),  // Halfway between white and final blue
                               array(0x02, 0x38, 0x82));
    $this->pdf->SetTextColor(0xf7, 0xd1, 0x17);
    $this->pdf->SetFont('Arial', 'B', 8);
    $this->pdf->Text($x + 0.95, $y + 1.65, 'Registed Awana Grand Prix Racer');
    $this->pdf->SetTextColor(0);

    // Watermark over everything, including the photo
    /*$this->pdf->SetAlpha(0.05);
    $this->pdf->Image(dirname(__FILE__).'/yellow_racing_car.png',
                      $x + 0.125, $y + 0.125, 3.25);
    $this->pdf->SetAlpha(1);
    $this->DrawBannerText($x, $y);*/
  }

 /* // The "banner" comprises a blue horizontal line, with small lines above and below.
  protected function DrawBanner($x, $y) {
    $this->pdf->SetFillColor(0x02, 0x38, 0x82);

    $this->pdf->Rect($x + 0.125, $y + 0.125, 3.25, 0.010, 'F');
    $this->pdf->Rect($x + 0.125, $y + 0.150, 3.25, 0.010, 'F');

    $this->pdf->Rect($x + 0.125, $y + 0.175, 3.25, 0.180, 'F');

    $this->pdf->Rect($x + 0.125, $y + 0.375, 3.25, 0.010, 'F');
    $this->pdf->Rect($x + 0.125, $y + 0.400, 3.25, 0.010, 'F');
  }*/

  // The "banner" is comprised of three images and a line.
  protected function DrawBanner($x, $y) {
    $this->pdf->Image(dirname(__FILE__).'/banner.png', $x, $y, 3);
    $this->pdf->SetFillColor(0x00, 0x00, 0x00);
    $this->pdf->Rect($x, $y + 0.50, 3.0, 0.01, 'F');
    // Washout part of the banner for better visibility
    $this->pdf->SetAlpha(0.750);
    $this->pdf->SetFillColor(0xFF, 0xFF, 0xFF);
    $this->pdf->Rect($x + 0.925, $y + 0.05, 1.15, 0.4, 'F');
    $this->pdf->SetAlpha(0.50);
    $this->pdf->Rect($x + 0.975, $y + 0.1, 1.05, 0.3, 'F');
    $this->pdf->SetAlpha(1);
    $this->pdf->CenteredSequence($x + 1.5, $y + 0.225,
                               array(new SetFontTag('Courier', 'I', 9), 'Official'));
    $this->pdf->CenteredSequence($x + 1.5, $y + 0.350,
                               array(new SetFontTag('Courier', 'I', 8), "Driver's License"));

    $this->pdf->Image(dirname(__FILE__).'/grand-prix-logo-color.png', $x + 0.05, $y + 0.025, 0.75);
    $this->pdf->Image(dirname(__FILE__).'/grand-prix-logo-color.png', $x + 2.20, $y + 0.025, 0.75);
  }

  protected function DrawBannerText($x, $y) {
    $this->pdf->SetFont('Arial', 'B', 10);
    $this->pdf->SetTextColor(0xf7, 0xd1, 0x17);
    $this->pdf->Text($x + 0.175, $y + 0.305, 'Pinewood Derby Racing License');
    $this->pdf->SetTextColor(0);
  }

  protected function ordinal($number) {
    $ends = array('th','st','nd','rd','th','th','th','th','th','th');
    if ((($number % 100) >= 11) && (($number % 100) <= 13))
      return $number.'th';
    else
      return $number.$ends[$number % 10];
  }
}
