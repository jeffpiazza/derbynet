<?php

// This sequencer class draws sequences of text interspersed with tag objects
// that may adjust how drawing is performed.
//
// DrawSequence($x, $y, $sequence)
//    Renders the sequence, starting at the indicated location.
//
// CenteredSequence($cx, $y, $sequence)
//    Horizontally centers the sequence at $cx.

// A Tag provides for modifying the current drawing environment
interface Tag {
  public function Apply(&$pdf);
}

class PDF_Sequencer extends PDF_Barcode {
  function DrawSequence($x, $y, $sequence) {
    foreach ($sequence as $item) {
      if (is_string($item)) {
        $this->Text($x, $y, $item);
        $x += $this->GetStringWidth($item);
      } else {
        $item->Apply($this);
      }
    }
  }

  function CenteredSequence($cx, $y, $sequence) {
    $len = 0;
    foreach ($sequence as $item) {
      if (is_string($item)) {
        $len += $this->GetStringWidth($item);
      } else {
        $item->Apply($this);
      }
    }

    $this->DrawSequence($cx - $len / 2, $y, $sequence);
  }
}

class SetFontTag implements Tag {
  private $font_name;
  private $weight;
  private $size;
  public function __construct($font_name, $weight, $size) {
    $this->font_name = $font_name;
    $this->weight = $weight;
    $this->size = $size;
  }
  public function Apply(&$pdf) {
    $pdf->SetFont($this->font_name, $this->weight, $this->size);
  }
}

?>