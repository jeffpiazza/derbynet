<?php

require_once('inc/photo-config.inc');

function make_new_document() {
  return new DriversLicenseDocument();
}

class DriversLicenseDocument extends PrintableRacerDocument {
  protected $count;

  function __construct() {
    parent::__construct();
    $this->count = 0;
  }

  public function DrawOne(&$racer) {
    $on_page = $this->count % 9;
    if ($on_page == 0) {
      $this->pdf->AddPage();
      $this->pdf->SetMargins(0, 0, 0);
      $this->pdf->SetAutoPageBreak(false);
    }

    $row = floor($on_page / 3);
    $col = $on_page % 3;
    
    $this->DrawOneAt(0.25 + $col * 3.6, 0.25 + $row * 2.6, $racer);
    ++$this->count;
  }

  protected function DrawBanner($x, $y) {
    $this->pdf->SetFillColor(0x02, 0x38, 0x82);
    $this->pdf->Rect($x + 0.125, $y + 0.125, 3.25, 0.35, 'F');
  }

  protected function DrawBannerText($x, $y) {
    $this->pdf->SetFont('Arial', 'B', 10);
    $this->pdf->SetTextColor(0xf7, 0xd1, 0x17);
    $this->pdf->Text($x + 0.175, $y + 0.295, 'Pinewood Derby Racing License');
    $this->pdf->SetTextColor(0);
  }

  protected function DrawOneAt($x, $y, &$racer) {
    $this->pdf->RoundedRect($x, $y, 3.5, 2.5, 0.125);

    $this->DrawBanner($x, $y);
    // We write the banner text at the end, over the top of the photo
    // $this->DrawBannerText($x, $y);

    $this->pdf->DrawSequence($x + 0.25, $y + 0.7,
                             array(new SetFontTag('Times', '', 12),
                                   iconv('UTF-8', 'windows-1252', $racer['firstname'])." ",
                                   new SetFontTag('Times', 'B', 12),
                                   iconv('UTF-8', 'windows-1252', $racer['lastname'])));

    $this->pdf->SetFont('Times', '', 10);
    $this->pdf->Text($x + 0.25, $y + 2.25, 'Expires '.date('l, F j, Y', time() + 24*60*60));
    
    if ($racer['imagefile']) {
      $this->pdf->Image($racer['imagefile'], $x + 2.25, $y + 0.25, 1.00);
    } else {
      $this->pdf->SetFont('Arial', '', 10);
      $this->pdf->SetXY($x + 2.25, $y + 0.25);
      $this->pdf->SetFillColor(0x80);
      $this->pdf->Cell(1.00, 1.33, "No photo", /* frame */ 1,
                       /* ln */ 0, /* align */ 'C', /* fill */ true);
    }

    // Watermark over everything, including the photo
    $this->pdf->SetAlpha(0.05);
    $this->pdf->Image(dirname($_SERVER['SCRIPT_FILENAME']).'/img/cubscout_emblem_800.png',
                      $x + 1.0, $y, 2.5, 2.5);
    $this->pdf->SetAlpha(1);
    $this->DrawBannerText($x, $y);
  }
}