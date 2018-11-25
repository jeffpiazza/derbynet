<?php

require_once('inc/photo-config.inc');

class SimpleAwardDocument extends PrintableAwardDocument {
  function name() { return "Simple Award Certificate"; }
  function get_available_options() {
    return array(
      'event_title' => array('type' => 'string',
                             'desc' => 'Event Title',
                             'default' => 'The '.date('Y').' Pinewood Derby'),
      'signatory1' => array('type' => 'string',
                             'desc' => 'First Signatory',
                            'default' => 'Cubmaster'),
      'signatory2' => array('type' => 'string',
                             'desc' => 'Second Signatory',
                            'default' => 'Pinewood Derby Chair'),
      'rounded' => array('type' => 'bool',
                         'desc' => 'Use rounded corners',
                         'default' => true),
        );
  }

  function StartDocument() {
    $this->initialize_pdf('L', 'in', 'Letter');
    $this->initialize_layout(10.0, 7.5);
//    $this->layout->SetHorizontalGap(0.1);
//    $this->layout->SetVerticalGap(0.1);
  }

  // $award argument contains these keys:
  //
  // awardid
  // awardname
  // awardtype
  // classid
  // rankid
  // racerid
  // carnumber, firstname, lastname, carname, class, rank, imagefile, carphoto
  //
  function DrawOneAt($left, $top, &$award) {
    $cx = $left + 5.0;
    $cy = $top + 3.75;

    $this->pdf->AddFont('BlackChancery', '', 'black-chancery.php');

    $this->pdf->SetLineWidth(0.05);
    if (true) {  // Rounded rects
      $this->pdf->SetDrawColor(2, 56, 130);  // blue
      $this->pdf->RoundedRect($left + 0.125, $top + 0.125, 9.75, 7.25, 0.125);
      $this->pdf->RoundedRect($left, $top, 10.0, 7.5, 0.25);
      $this->pdf->SetDrawColor(247, 209, 23);  // gold
      $this->pdf->RoundedRect($left + 0.0625, $top + 0.0625, 9.875, 7.375, 0.1875);
    } else {
      $this->pdf->SetDrawColor(2, 56, 130);  // blue
      $this->pdf->Rect($left + 0.125, $top + 0.125, 9.75, 7.25);
      $this->pdf->Rect($left, $top, 10.0, 7.5);
      $this->pdf->SetDrawColor(247, 209, 23);  // gold
      $this->pdf->Rect($left + 0.0625, $top + 0.0625, 9.875, 7.375);
    }

    $this->pdf->Image(dirname(__FILE__).'/rachel-davis-logo.png',
                      $cx - 3.0/2, $top + 0.200, 3.0);

    $this->pdf->SetFont('BlackChancery', '', 30);

    // Award name
    $this->pdf->CenteredSequence($cx, $top + 2.5, array($award['awardname']));

    // Recipient
    if (isset($award['firstname'])) {
      $this->pdf->SetFont('BlackChancery', '', 50);
      $this->pdf->CenteredSequence($cx, $cy, array($award['firstname'].' '.$award['lastname']));
    } else {
      $this->pdf->SetLineWidth(0.01);
      $this->pdf->SetDrawColor(0);
      $this->pdf->Line($cx - 3.0, $cy, $cx + 3.0, $cy);
    }

    // TODO Car number?  Car name?

    $this->pdf->SetFont('Times', '', 15);
    $this->pdf->SetLineWidth(0.01);
    $this->pdf->SetDrawColor(0);

    // Signature lines:
    if ($this->get_option('signatory1')) {
      $this->pdf->CenteredSequence($left + 0.1875 + 1.5, $top + 6.0, array($this->get_option('signatory1')));
      $this->pdf->Line($left + 0.1875, $top + 6.5, $left + 0.1875 + 3.0, $top + 6.5);
    }
    if ($this->get_option('signatory2')) {
      $this->pdf->CenteredSequence($left + 10.0 - 0.1875 - 1.5, $top + 6.0, array($this->get_option('signatory2')));
      $this->pdf->Line($left + 10.0 - 0.1875 - 3.0, $top + 6.5, $left + 10.0 - 0.1875, $top + 6.5);
    }

    // Event Title
    if ($this->get_option('event_title')) {
      $this->pdf->SetFont('Helvetica', '', 16);
      $this->pdf->CenteredSequence($cx, $top + 7.2, array($this->get_option('event_title')));
    }
  }
}

?>