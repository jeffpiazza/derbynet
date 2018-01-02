<?php

// Some utility methods for rotating text and images

class PDF_RotationAndCentering extends PDF_Sequencer
{
  // ** These are unused and untested: **

//  function RotatedText($x,$y,$txt,$angle)
//  {
//    //Text rotated around its origin
//    $this->StartTransform();
//    $this->Rotate($angle,$x,$y);
//    $this->Text($x,$y,$txt);
//    $this->StopTransform();
//  }

//  function RotatedImage($file,$x,$y,$w,$h,$angle)
//  {
//    //Image rotated around its upper-left corner
//    $this->StartTransform();
//    $this->Rotate($angle,$x,$y);
//    $this->Image($file,$x,$y,$w,$h);
//    $this->StopTransform();
//  }

//  function RotatedCenteredCell($w, $h, $txt, $angle) {
//    $x = $this->GetX() + $h/2;
//    $y = $this->GetY() + $w/2;
//    $this->StartTransform();
//    $this->Rotate($angle, $x, $y);
//    $this->Cell($h, $w, $txt, /* border */ 0, /* ln */ 0, "C");
//    $this->StopTransform();
//  }

  // $cx describes the horizontal center, $y the vertical text location.
  function RotatedCenteredText($cx, $y, $txt) {
    $this->StartTransform();
    $this->Rotate(90, $cx, $cy);
    $this->CenteredSequence($cx, $cy, array($txt));
    $this->StopTransform();
  }

}
?>