<?php

require_once('inc/partitions.inc');
require_once('inc/photo-config.inc');

class AwanaDriversLicenseDocument extends PrintableRacerDocument {
  function name() { return "Driver's License - AWANA"; }

  function get_available_options() {
    return array(
      'photo_size' => array('type' => 'bool',
                           'desc' => '4x6 photo size',
                           'default' => true),
      'use_photo' => array('type' => 'bool',
                           'desc' => 'Include racer\'s photo',
                           'default' => true),
      'use_barcode' => array('type' => 'bool',
                             'desc' => 'Include barcode',
                             'default' => false),
      'date_of_issue' => array('type' => 'string',
                               'desc' => 'Date of Issue',
                               'default' => date('F j, Y')),
      );
  }

  function StartDocument() {
    if ($this->get_option('photo_size')) {
      $this->initialize_pdf('P', 'in', array(4,6));
    } else {
      $this->initialize_pdf('L', 'in', 'Letter');
    }
    $this->initialize_layout(3.0, 1.75);
    $this->layout->SetHorizontalGap(0.1);
    $this->layout->SetVerticalGap(0.1);
  }

  function DrawOneAt($x, $y, &$racer) {
    $issue_date = new DateTimeImmutable($this->get_option('date_of_issue'));
    
    $this->DrawBanner($x, $y);
    $this->pdf->RoundedRect($x, $y, 3, 1.75, 0.1);

    // Washout the background for better visibility
    $this->pdf->SetAlpha(0.15);
    $this->pdf->Image(dirname(__FILE__).'/awana-clubs-logo-color.jpg', $x + 0.90, $y + 0.60, 2);
    $this->pdf->SetAlpha(1);

    if (!$this->get_option('use_photo')) {
    } else if ($racer['imagefile']) {
      $this->pdf->Image($racer['imagefile'], $x + 0.1, $y + 0.6, 0.75);
    } else {
      $this->pdf->Image(dirname(__FILE__).'/yellow_racing_car.png', $x + 0.1, $y + 0.75, 0.75);
    }

    $this->pdf->DrawSequence($x + 2.80, $y + 0.60,
                               array(new SetFontTag('Times', 'B', 8), str_replace("PWDid", "", $racer['barcode'])));

    $this->pdf->SetFillColor(0);

    // If using barcode
    if ($this->get_option('use_barcode')) {
      $this->pdf->HorizontalBarcode($x + 0.90, $y + 1.15, 2.0, 0.20, $racer['barcode']);
    } else {
    
    $this->pdf->DrawSequence($x + 0.10, $y + 0.59,
                             array(new SetFontTag('Times', 'B', 5), 'Don\'t text and drive. Don\'t drink and drive. Don\'t pop bubble-wrap and drive. ',
                                   new SetFontTag('Times', 'BI', 5), 'Have fun!'));
      $y = $y + 0.1;
    }

    $this->pdf->DrawSequence($x + 0.90, $y + 0.75,
                             array(new SetFontTag('Times', 'B', 14), $racer['firstname']." ",
                                   new SetFontTag('Times', 'B', 14), $racer['lastname']));

    if (use_groups()) {
      $this->pdf->DrawSequence($x + 0.90, $y + 0.95,
                               array(new SetFontTag('Times', '', 10), group_label().": ",
                                     new SetFontTag('Times', 'B', 10), $racer['class']));
    }

    $this->pdf->CenteredSequence($x + 2.35, $y + 1.075,
                                 array(new SetFontTag('Arial', 'B', 24),
                                       strval($racer['carnumber'])));

    if (!$this->get_option('use_barcode')) {
      $y = $y - 0.15;
    }
    
    $this->pdf->DrawSequence($x + 0.90, $y + 1.5,
                             array(new SetFontTag('Times', '', 10), 'Issued on the ',
                                   new SetFontTag('Times', '', 10),
                                   $this->ordinal(1 + $issue_date->format('z')), ' day of ',
                                   $issue_date->format('Y')));
    if (!$this->get_option('use_barcode')) {
      $y = $y + 0.05;
    }

    $this->pdf->SetFillColor(0x02, 0x38, 0x82);
    $this->pdf->Rect($x + 0.5, $y + 1.535, 2.5, 0.15, 'F');
    $this->pdf->LinearGradient($x + 0.5, $y + 1.535, 1.5, 0.15,
                               array(129, 155, 192),  // Halfway between white and final blue
                               array(0x02, 0x38, 0x82));
    $this->pdf->SetTextColor(0xf7, 0xd1, 0x17);
    $this->pdf->SetFont('Arial', 'B', 8);
    $this->pdf->Text($x + 0.95, $y + 1.65, 'Registered Awana Grand Prix Racer');
    $this->pdf->SetTextColor(0);
  }

  // The "banner" is comprised of a checked background, two images, and a line.
  protected function DrawBanner($x, $y) {
    //$this->pdf->Image(dirname(__FILE__).'/banner.png', $x, $y, 3);
    $this->DrawBannerBackground($x, $y);
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

// assemble the checkered background of the top banner
  protected function DrawBannerBackground($x, $y) {
    $this->pdf->SetFillColor(0x00, 0x00, 0x00);
    // assemble the main body of the banner background
    for($i=0.0; $i <= 0.4; $i+=0.1) {
      for($j=0.025; $j <= 2.9; $j+=0.1) {
        $this->pdf->Rect($x+$j, $y+$i+0.05, 0.05, 0.05, 'F');
        $this->pdf->Rect($x+$j+0.05, $y+$i, 0.05, 0.05, 'F');
      }
      $this->pdf->Rect($x+2.925, $y+$i+0.05, 0.05, 0.05, 'F');
    }
    // place the half-squares along the sides
    for($i=0.1; $i <= 0.4; $i+=0.1) {
      $this->pdf->Rect($x, $y+$i, 0.025, 0.05, 'F');
      $this->pdf->Rect($x+2.975, $y+$i, 0.025, 0.05, 'F');
    }
    // place the tiny square missing from the upper corners
    $this->pdf->Rect($x+0.015, $y+0.04, 0.01, 0.01, 'F');
    $this->pdf->Rect($x+2.975, $y+0.04, 0.01, 0.01, 'F');
  }

  protected function ordinal($number) {
    $ends = array('th','st','nd','rd','th','th','th','th','th','th');
    if ((($number % 100) >= 11) && (($number % 100) <= 13))
      return $number.'th';
    else
      return $number.$ends[$number % 10];
  }
}
