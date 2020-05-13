<?php

// Print 1-3/4 x 2/3 labels, compatible with Avery 5195 and the like.
class CarLabel5195 extends PrintableRacerDocument {
  function name() { return "Car Label 5195"; }

  function get_available_options() {
    return array('skip' => array('type' => 'int',
                                 'desc' => 'Leave this many labels blank, for printing on a'
                                            .' partially-used sheet.',
                                 'default' => 0),
                 'outline' => array('type' => 'bool',
                                    'desc' => 'Draw outline around each label.',
                                    'default' => false),
                 'show_class' => array('type' => 'bool',
                                       'desc' => 'Include '.group_label_lc(),
                                       'default' => true),
                 'show_rank' => array('type' => 'bool',
                                      'desc' => 'Include '.subgroup_label_lc(),
                                      'default' => use_subgroups()),
                   );
  }

  function StartDocument() {
    $this->initialize_pdf('P', 'in', 'Letter');
    $this->initialize_layout(1.750, 0.660);
    $this->layout->SetHorizontalGap(0.300);
    $this->layout->SetTopMargin(0.550);

    $this->nskip = $this->get_option('skip');
    $this->draw_outline = $this->get_option('outline');
    $this->show_class = $this->get_option('show_class');
    $this->show_rank = $this->get_option('show_rank');
  }

  private $nskip;
  private $draw_outline;
  private $show_class;
  private $show_rank;

  function DrawOneAt($left, $top, &$racer) {
    if ($this->nskip > 0) {
      while ($this->nskip > 0) {
        if ($this->draw_outline) {
          $this->pdf->RoundedRect($left, $top, 1.750, 0.660, 0.125);
        }
        $xy = $this->layout->NextTopLeft();
        $left = $xy['x'];
        $top = $xy['y'];
        --$this->nskip;
      }
    }

    if ($this->draw_outline) {
      $this->pdf->RoundedRect($left, $top, 1.750, 0.660, 0.125);
    }
    
    $cx = $left + 1.75/2;

    $this->pdf->CenteredSequence($cx, $top + 0.2,
                                 array(new SetFontTag('Times', '', 12), $racer['firstname']." ",
                                       new SetFontTag('Times', 'B', 12), $racer['lastname']));
    $this->pdf->CenteredSequence($cx, $top + 0.42,
                                 array(new SetFontTag('Helvetica', 'B', 16), $racer['carnumber']));

    if ($this->show_class || $this->show_rank) {
      $category = "";
      if ($this->show_class) {
        $category = $racer['class'];
      }
      if ($this->show_class && $this->show_rank) {
        $category .= " / ";
      }
      if ($this->show_rank) {
        $category .= $racer['rank'];
      }
      $this->pdf->CenteredSequence($cx, $top + 0.58,
                                   array(new SetFontTag('Times', '', 11), $category));
    }
  }
}
?>