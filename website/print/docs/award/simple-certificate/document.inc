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
        );
  }

  function StartDocument() {
    $this->initialize_pdf('L', 'in', 'Letter');
    $this->initialize_layout(10.0, 7.5);
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
    $params = $this->layout->GetParameters()['doc'];
    $cx = $left + $params['width']/2;
    $cy = $top + $params['height']/2;

    $this->pdf->AddFont('BlackChancery', '', 'black-chancery.php');

    $this->pdf->SetLineWidth(0.05);
    $this->pdf->SetDrawColor(2, 56, 130);  // blue
    $this->pdf->RoundedRect($left + 0.125, $top + 0.125, 9.75, 7.25, 0.125);
    $this->pdf->RoundedRect($left, $top, 10.0, 7.5, 0.25);
    $this->pdf->SetDrawColor(247, 209, 23);  // gold
    $this->pdf->RoundedRect($left + 0.0625, $top + 0.0625, 9.875, 7.375, 0.1875);

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

    $this->pdf->SetFont('Times', '', 15);
    $this->pdf->SetLineWidth(0.01);
    $this->pdf->SetDrawColor(0);

    // Signature lines:
    $sig_width = 3.0;   // Length of each signature line
    $sig_margin = 0.1875;  // Margin from signature line to the nearest horizontal edge
    
    if ($this->get_option('signatory1')) {
      $this->pdf->CenteredSequence($left + $sig_margin + $sig_width / 2, $top + 6.0,
                                   array($this->get_option('signatory1')));
      $this->pdf->Line($left + $sig_margin, $top + 6.5,
                       $left + $sig_margin + $sig_width, $top + 6.5);
    }
    if ($this->get_option('signatory2')) {
      $this->pdf->CenteredSequence($left + $params['width'] - $sig_margin - $sig_width/2, $top + 6.0,
                                   array($this->get_option('signatory2')));
      $this->pdf->Line($left + $params['width'] - $sig_margin - $sig_width, $top + 6.5,
                       $left + $params['width'] - $sig_margin,              $top + 6.5);
    }

    // Event Title
    if ($this->get_option('event_title')) {
      $this->pdf->SetFont('Helvetica', '', 16);
      $this->pdf->CenteredSequence($cx, $top + 7.2, array($this->get_option('event_title')));
    }
  }
}

?>