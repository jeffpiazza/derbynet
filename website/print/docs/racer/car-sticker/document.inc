<?php

class CarSticker extends PrintableRacerDocument {
  function name() { return "Car Stickers"; }

  function get_available_options() {
    return array('sheet' => array('type' => 'radio',
                                  'desc' => 'What kind',
                                  'values' =>
                                  array(array('value' => 'avery-5195',
                                              'desc' => 'Avery 5195'),
                                        array('value' => 'avery-5167',
                                              'desc' => 'Avery 5167'),
                                        array('value' => 'avery-5408',
                                              'desc' => 'Avery 5408'),
                                        array('value' => 'pup',
                                              'desc' => 'Garage Sale Pup 1/2-inch round'))),
                 'skip' => array('type' => 'int',
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
    $this->nskip = $this->get_option('skip');
    $this->draw_outline = $this->get_option('outline');
    $this->show_class = $this->get_option('show_class');
    $this->show_rank = $this->get_option('show_rank');

    $this->set_parameters($this->get_option('sheet'));

    if ($this->get_option('sheet') == 'avery-5408'){
      $this->initialize_pdf('P', 'in', array(4, 6));
    }else{
     $this->initialize_pdf('P', 'in', 'Letter');
    }
    $this->initialize_layout($this->params['doc_width'], $this->params['doc_height']);
    $this->AdjustLayout();
  }

  protected $params;

  private $nskip;
  private $draw_outline;

  protected $show_class;
  protected $show_rank;
  protected $show_name = true;

  function set_parameters($sheet) {
    if ($sheet == 'avery-5195') {
        $this->params = array(
          'doc_width' => 1.750,
          'doc_height' => 0.660,
          'x_gap' => 0.300,
          'top_margin' => 0.550,
          'name_y' => 0.2,
          'name_font_size' => 12,
          'carno_y' => 0.42,
          'carno_font_size' => 16,
          'category_y' => 0.58,
          'category_font_size' => 11
          );
    } else if ($sheet == 'avery-5167') {
      $this->params = array(
        'doc_width' => 1.750,
        'doc_height' => 0.500,
        'x_gap' => 0.300,
        'top_margin' => 0.500,
        'name_y' => 0.135,
        'name_font_size' => 9,
        'carno_y' => 0.325,
        'carno_font_size' => 14,
        'category_y' => 0.44,
        'category_font_size' => 9
        );
    } else if ($sheet == 'avery-5408') {
      $this->params = array(
        'doc_width' => 0.750,
        'doc_height' => 0.750,
        'carno_y' => 0.45,
        'carno_font_size' => 16,
        );
      // These options are rendered moot:
      $this->show_name = false;
      $this->show_class = false;
      $this->show_rank = false;
    } else if ($sheet == 'pup') {
      $this->params = array(
        'doc_width' => 0.5,
        'doc_height' => 0.5,

        'carno_y' => 0.3,
        'carno_font_size' => 16,
        );
      // These options are rendered moot:
      $this->show_name = false;
      $this->show_class = false;
      $this->show_rank = false;
    }
  }

  protected function AdjustLayout() {
    if ($this->get_option('sheet') == 'pup') {
      $this->layout->SetRawLayoutParameters(/* top_margin*/0.45, /*left_margin*/0.48,
                                            /* hgap */0.14, /* vgap */0.1,
                                            /* n_per_row */12, /* n_per_page */204);
    } elseif ($this->get_option('sheet') == 'avery-5408'){
       $this->layout->SetRawLayoutParameters(/* top_margin*/0.6, /*left_margin*/0.3,
                                            /* hgap */0.13, /* vgap */0.06,
                                            /* n_per_row */4, /* n_per_page */24);
    } else {
      $this->layout->SetHorizontalGap($this->params['x_gap']);
      $this->layout->SetTopMargin($this->params['top_margin']);
    }
  }
  
  protected function DrawOutline($left, $top) {
    if ($this->get_option('sheet') == 'pup' || $this->get_option('sheet') == 'avery-5408') {
      $r = $this->params['doc_height'] / 2;  // = $this->params['doc_width'] / 2
      $this->pdf->Circle($left + $r, $top + $r, $r);
    } else {
      $this->pdf->RoundedRect($left, $top,
                              $this->params['doc_width'], $this->params['doc_height'],
                              /* corner radius */ 0.125);
    }
  }

  function DrawOneAt($left, $top, &$racer) {
    while ($this->nskip > 0) {
      if ($this->draw_outline) {
        $this->DrawOutline($left, $top);
      }
      $xy = $this->layout->NextTopLeft();
      $left = $xy['x'];
      $top = $xy['y'];
      --$this->nskip;
    }

    if ($this->draw_outline) {
      $this->DrawOutline($left, $top);
    }
    
    $cx = $left + $this->params['doc_width']/2;

    /* Show the car number */
    $this->pdf->CenteredSequence($cx, $top + $this->params['carno_y'],
                                 array(new SetFontTag('Helvetica', 'B', $this->params['carno_font_size']),
                                       strval($racer['carnumber'])));

    if ($this->show_name) {
      $this->pdf->CenteredSequence($cx, $top + $this->params['name_y'],
                                   array(new SetFontTag('Times', '', $this->params['name_font_size']),
                                         $racer['firstname']." ",
                                         new SetFontTag('Times', 'B', $this->params['name_font_size']),
                                         $racer['lastname']));
    }

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
      $this->pdf->CenteredSequence($cx, $top + $this->params['category_y'],
                                   array(new SetFontTag('Times', '', $this->params['category_font_size']),
                                         $category));
    }
  }
}
?>
