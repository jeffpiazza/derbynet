<?php
// Print 1/2 circular labels, compatible with GarageSalePup AWE.
class CarLabelAWE extends PrintableRacerDocument {
  function name() { return "Car Label GarageSalePup AWE"; }
  function get_available_options() {
    return array('skip' => array('type' => 'int',
                                 'desc' => 'Leave this many labels blank, for printing on a'
                                            .' partially-used sheet.',
                                 'default' => 0),
                 'outline' => array('type' => 'bool',
                                    'desc' => 'Draw outline around each label.',
                                    'default' => false)
                   );
  }
  function StartDocument() {
    $this->initialize_pdf('P', 'in', 'Letter');
    $this->initialize_layout(0.5, 0.5);
    $this->layout->SetRawLayoutParameters(0.45, 0.48, 0.14, 0.1, 12, 204);
    $this->nskip = $this->get_option('skip');
    $this->draw_outline = $this->get_option('outline');
  }
  private $nskip;
  private $draw_outline;
  function DrawOneAt($left, $top, &$racer) {
    if ($this->nskip > 0) {
      while ($this->nskip > 0) {
        if ($this->draw_outline) {
          $this->pdf->Circle($left+0.5/2, $top+0.5/2, 0.25);
        }
        $xy = $this->layout->NextTopLeft();
        $left = $xy['x'];
        $top = $xy['y'];
        --$this->nskip;
      }
    }
    if ($this->draw_outline) {
      $this->pdf->Circle($left+0.5/2, $top+0.5/2, 0.25);
    }
    
    $cx = $left + 0.5/2;
    $this->pdf->CenteredSequence($cx, $top + 0.3,
                                 array(new SetFontTag('Helvetica', 'B', 16), $racer['carnumber']));
  }
}
?>
