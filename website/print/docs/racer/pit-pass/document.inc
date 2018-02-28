<?php

class PitPassDocument extends PrintableRacerDocument {
  protected $count;

  function get_available_options() {
    return array(
//      'use_photo' => array('desc' => 'Include racer\'s photo',
//                           'default' => true),
//      'use_barcode' => array('desc' => 'Include barcode',
//                             'default' => true),
      );
  }

  function StartDocument() {
    $this->initialize_pdf('P', 'in', 'Letter');
    $this->count = 0;
  }

  public function DrawOne(&$racer) {
    $on_page = $this->count % 4;
    if ($on_page == 0) {
      $this->pdf->AddPage();
      $this->pdf->SetMargins(0, 0, 0);
      $this->pdf->SetAutoPageBreak(false);
    }

    $row = floor($on_page / 2);
    $col = $on_page % 2;

    // Each pass is 3.825 x 5.000
    $this->DrawOneAt(0.25 + $col * 4.25, 0.25 + $row * 5.5, $racer);
    ++$this->count;
  }

  protected function DrawOneAt($x, $y, &$racer) {
    if (isset($this->options['use_photo']) && !$this->options['use_photo']) {
    } else if ($racer['imagefile']) {
      // (3.825 - 2.5) / 2 = 1.325/2 = 0.6625
      $this->pdf->Image($racer['imagefile'], $x + 0.6625, $y + 0.6, 2.5);
    }

    $this->pdf->Image(dirname(__FILE__).'/pit-pass-frame.png', $x, $y, 3.825, 5.000);
  }
}

?>