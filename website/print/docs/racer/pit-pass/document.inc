<?php

class PitPassDocument extends PrintableRacerDocument {
  protected $count;

  function name() {
    return "Pit Pass";
  }

  function get_available_options() {
    return array(
      'with_logo' => array('type' => 'bool',
                           'desc' => 'Include Cub Scout logo in frame',
                           'default' => true),
      'event_title1' => array('type' => 'string',
                              'desc' => 'First line of event name, to be included in frame',
                              'default' => ''),
      'event_title2' => array('type' => 'string',
                              'desc' => 'Second line of event name, to be included in frame',
                              'default' => ''),
      );
  }

  protected $y_coords = array(
    'title-rect' => 0.10,
    'title1' => 0.22,
    'title2' => 0.36,
    'imagefile' => 0.6,
    'namebox' => 0.50,
    // Not really enough height to fit in the car name
    // on this version.
    'carname' => 0,
    'carnumber' => 3.82,
    'yellow-car' => 3.125,
    'date-rect' => 4.62,
    'date' => 4.82,
    );

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

  // $y is the top of the background box.
  protected function DrawNameBox($x, $y, &$racer) {
    $w = 0;
    $this->pdf->SetFont('Times', 'I', 18);
    $w += $this->pdf->GetStringWidth($racer['firstname']."m");
    $this->pdf->SetFont('Times', 'BI', 18);
    $w += $this->pdf->GetStringWidth($racer['lastname']);
    $w = $w + 0.1;
    // Cub Scout yellow background
    $this->pdf->SetFillColor(0xf7, 0xd1, 0x17);

    $this->pdf->Rect($x + (3.825 - $w) / 2, $y, $w, 20 / 72, 'F');

    // Cub Scout blue text
    $this->pdf->SetTextColor(0x02, 0x38, 0x82);
    $this->pdf->CenteredSequence($x + 3.825 / 2, $y + 0.22,
                                 array(new SetFontTag('Times', 'I', 18), $racer['firstname']." ",
                                       new SetFontTag('Times', 'BI', 18), $racer['lastname']));

  }

  protected function DrawBorder($x, $y) {
    if ($this->get_option('with_logo')) {
      $file_path = '/pit-pass-frame-logo.png';
    } else {
      $file_path = '/pit-pass-frame.png';
    }

    $this->pdf->Image(dirname(__FILE__).$file_path, $x, $y, 3.825, 5.000);
  }

  protected function DrawOneAt($x, $y, &$racer) {
    // Race car image goes underneath
    $this->pdf->SetAlpha(0.25);
    $this->pdf->Image(dirname(__FILE__).'/yellow_racing_car.png',
                      $x + (3.825 - 2.25) / 2,
                      $y + $this->y_coords['yellow-car'],
                      2.25);
    $this->pdf->SetAlpha(1);

    if (isset($this->options['use_photo']) && !$this->options['use_photo']) {
    } else if ($racer['imagefile']) {
      // (3.825 - 2.5) / 2 = 1.325/2 = 0.6625
      $this->pdf->Image($racer['imagefile'], $x + 0.6625, $y + $this->y_coords['imagefile'], 2.5);
    }

    $this->DrawBorder($x, $y);

    $this->DrawNameBox($x, $y + $this->y_coords['namebox'], $racer);

    $this->pdf->SetFont('Times', '', 16);
    $this->pdf->SetTextColor(0, 0, 64);
    if ($this->y_coords['carname']) {
      $this->pdf->CenteredText($x + 3.825/2, $y + $this->y_coords['carname'], $racer['carname']);
    }

    $this->pdf->SetFont('Helvetica', 'BI', 20);
    $this->pdf->SetTextColor(0, 0, 0);
    $this->pdf->CenteredText($x + 3.825/2, $y + $this->y_coords['carnumber'], $racer['carnumber']);

    // Event title in top border
    $event_title1 = $this->get_option('event_title1');
    $event_title2 = $this->get_option('event_title2');
    if ($event_title1 != '') {
      $this->pdf->SetFillColor(240, 240, 240);
      $w = 1.5;  // Width of background box for event title
      $this->pdf->Rect($x + (3.825 - $w) / 2, $y + $this->y_coords['title-rect'], $w, 0.30, 'F'); 
      $this->pdf->SetFont('Helvetica', 'BI', 10);
      $this->pdf->SetTextColor(128, 0, 0);

      $this->pdf->CenteredText($x + 3.825/2, $y + $this->y_coords['title1'], $event_title1);
      $this->pdf->CenteredText($x + 3.825/2, $y + $this->y_coords['title2'], $event_title2);
    }

    // Date in bottom border
    $this->pdf->SetFillColor(240, 240, 240);
    $w = 1.5;  // Width of background box for event date
    $this->pdf->Rect($x + (3.825 - $w) / 2, $y + $this->y_coords['date-rect'], $w, 0.30, 'F'); 
    $this->pdf->SetFont('Helvetica', 'BI', 10);
    $this->pdf->SetTextColor(128, 0, 0);
    $this->pdf->CenteredText($x + 3.825/2, $y + $this->y_coords['date'], date('F j, Y'));
  }
}

class PitPass4x6 extends PitPassDocument {

  function name() { return "Pit Pass 4x6"; }

  protected $y_coords = array(
    'title-rect' => 0.10,
    'title1' => 0.22,
    'title2' => 0.36,
    'imagefile' => 0.6,
    'namebox' => 3.5,
    'carname' => 4.02,
    'carnumber' => 4.35,
    'yellow-car' => 4.125,
    'date-rect' => 5.62,
    'date' => 5.82,
    );

  public function StartDocument() {
    $this->initialize_pdf('P', 'in', array(4, 6));
    $this->count = 0;
  }

  protected function DrawBorder($x, $y) {
    if ($this->get_option('with_logo')) {
      $file_path = '/pit-pass-frame-4x6-logo.png';
    } else {
      $file_path = '/pit-pass-frame-4x6.png';
    }

    $this->pdf->Image(dirname(__FILE__).$file_path, $x, $y, 3.825, 6.000);
  }

  public function DrawOne(&$racer) {
    $this->pdf->AddPage();
    $this->pdf->SetMargins(0, 0, 0);
    $this->pdf->SetAutoPageBreak(false);

    // Each pass is 3.825 x 5.000
    $this->DrawOneAt(0.0875, 0, $racer);
    ++$this->count;
  }
}

?>