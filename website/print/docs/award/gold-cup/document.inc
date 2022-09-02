<?php

require_once('inc/photo-config.inc');
require_once('inc/path-info.inc');

class GoldCupAwardDocument extends PrintableAwardDocument {
  function name() { return "Gold Cup Award Certificate"; }
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
      'logo' => array('type' => 'bool',
                      'desc' => 'Include logo in corner',
                      'default' => true),
        );
  }

  function StartDocument() {
    $this->initialize_pdf('L', 'in', 'Letter');
    $this->initialize_layout(11.0, 8.5);
  }

  protected function DrawUnderlay($x, $y, $w, $h) {
    $this->pdf->SetAlpha(0.75);
    $this->pdf->SetFillColor(255, 255, 255);
    $this->pdf->RoundedRect($x, $y, $w, $h, 0.125, 'F');
    $this->pdf->SetAlpha(1);
  }

  // Horizontally-centered underlay
  protected function DrawCenteredUnderlay($cx, $y, $w, $h) {
    $this->DrawUnderlay($cx - $w / 2, $y, $w, $h);
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

    $this->pdf->Image(dirname(__FILE__).'/gold-cup-certificate.png',
                      $left, $top, $params['width']);

    $this->pdf->SetFont('BlackChancery', '', 30);

    // Award name
    $award_name_width = $this->pdf->GetStringWidth($award['awardname']);
    $this->DrawCenteredUnderlay($cx, $top + 2.0, $award_name_width + 1.0, 0.75);
    $this->pdf->CenteredSequence($cx, $top + 2.5, array($award['awardname']));

    $this->pdf->SetLineWidth(0.01);
    // Recipient
    if (isset($award['firstname'])) {
      $this->pdf->SetFont('BlackChancery', '', 50);
      $recip = $award['firstname'].' '.$award['lastname'];
      $recip_width = $this->pdf->GetStringWidth($recip);
      $recip_width += 1.0;
      $this->DrawCenteredUnderlay($cx, $cy - 0.5, $recip_width, 1.0);
      $this->pdf->CenteredSequence($cx, $cy, array($recip));
    } else {
      $this->pdf->SetDrawColor(0);
      $recip_width = 6.0;
      $this->DrawCenteredUnderlay($cx, $cy - 0.75, $recip_width + 1.0, 1.0);
      $this->pdf->Line($cx - $recip_width / 2, $cy, $cx + $recip_width / 2, $cy);
    }

    $this->pdf->SetFont('Times', '', 15);
    $this->pdf->SetLineWidth(0.01);
    $this->pdf->SetDrawColor(0);

    // Signature lines:
    $sig_width = 3.0;   // Length of each signature line
    $sig_margin = 0.85;  // Margin from signature line to the nearest horizontal edge
    
    if ($this->get_option('signatory1')) {
      $this->pdf->CenteredSequence($left + $sig_margin + $sig_width / 2,
                                   $top + 6.0, array($this->get_option('signatory1')));
      $this->pdf->Line($left + $sig_margin, $top + 6.5,
                       $left + $sig_margin + $sig_width, $top + 6.5);
    }
    if ($this->get_option('signatory2')) {
      $this->pdf->CenteredSequence($left + $params['width'] - $sig_margin - $sig_width/2, $top + 6.0,
                                   array($this->get_option('signatory2')));
      $this->pdf->Line($left + $params['width'] - $sig_margin - $sig_width, $top + 6.5,
                       $left + $params['width'] - $sig_margin,              $top + 6.5);
    }

    if ($this->get_option('logo')) {
      $path = image_file_path('emblem');
      $this->pdf->Image($path, $left + 0.25, 6.75, 1.5,1.5);
    }

    // Event Title
    if ($this->get_option('event_title')) {
      $this->pdf->SetFont('Helvetica', '', 20);
      $this->pdf->CenteredSequence($cx, $top + 7.5, array($this->get_option('event_title')));
    }
  }
}

?>
