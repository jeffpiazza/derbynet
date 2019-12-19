<?php
require_once('print/inc/layout.inc');
require_once('print/inc/load_document_files.inc');

abstract class PrintableRacerDocument {
  protected $pdf;
  protected $layout;
  protected $options;  // Contents vary by document type

  // Returns an array of key => {type:, desc:, default:}, with an entry for each
  // option key understood by the document class.
  //
  // type can be one of 'bool', 'string', 'int'.
  abstract function get_available_options();

  // Returns a name for the document class
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

  // $racer argument contains these keys:
  //
  // racerid
  // firstname
  // lastname
  // carnumber
  // carname
  // class
  // classid
  // rank
  // rankid
  // imagefile
  // carphoto
  // barcode
  abstract function DrawOneAt($left, $top, &$racer);

  public function DrawOne(&$racer) {
    $xy = $this->layout->NextTopLeft();
    $this->DrawOneAt($xy['x'], $xy['y'], $racer);
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
                         . DIRECTORY_SEPARATOR . 'racer');
?>