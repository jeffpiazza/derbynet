<?php
require_once('print/inc/layout.inc');
require_once('print/inc/load_document_files.inc');

abstract class PrintableAwardDocument {
  protected $pdf;
  protected $layout;
  protected $options;  // Contents vary by document type

  abstract function get_available_options();
  abstract function name();

  function set_options($options) {
    $this->options = $options;
  }

  function get_option($option) {
    if (isset($this->options[$option]))
      return $this->options[$option];
    $opts = $this->get_available_options();
    return $opts[$option]['default'];
  }

  abstract function StartDocument();

  // $award argument contains these keys:
  //
  // awardid
  // awardname
  // awardtype
  // classid
  // rankid
  // racerid
  //
  abstract function DrawOneAt($left, $top, &$award);

  public function DrawOne(&$award) {
    $xy = $this->layout->NextTopLeft();
    $this->DrawOneAt($xy['x'], $xy['y'], $award);
  }

  public function Output() {
    $this->pdf->Output();
  }

  protected function initialize_pdf($orientation, $unit, $size) {
    $this->pdf = new PDF_Combined($orientation, $unit, $size);
  }

  protected function initialize_layout($doc_width, $doc_height) {
    $this->layout = new DocumentLayout($this->pdf, $doc_width, $doc_height);
  }
}

load_document_files_from(dirname(dirname(__FILE__))
                         . DIRECTORY_SEPARATOR . 'docs'
                         . DIRECTORY_SEPARATOR . 'award');
?>