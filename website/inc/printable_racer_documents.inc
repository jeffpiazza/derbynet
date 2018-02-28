<?php

function load_document_files_from($dir) {
  if (is_dir($dir)) {
    if ($dh = opendir($dir)) {
      while (($subdir = readdir($dh)) !== false) {
        if ($subdir !== "." && $subdir !== ".." && is_dir($dir . DIRECTORY_SEPARATOR . $subdir)) {
          require_once( $dir . DIRECTORY_SEPARATOR . $subdir . DIRECTORY_SEPARATOR . "document.inc" );
        }
      }
      closedir($dh);
    }
  }
}

abstract class PrintableRacerDocument {
  protected $pdf;
  protected $options;  // Contents vary by document type

  // Returns an array of key => {desc:, default:}, with an entry for each option key
  // understood by the document class.
  abstract function get_available_options();

  function set_options($options) {
    $this->options = $options;
  }

  abstract function StartDocument();

  // Argument contains these keys:
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
  abstract public function DrawOne(&$racer);

  public function Output() {
    $this->pdf->Output();
  }

  protected function initialize_pdf($orientation, $unit, $size) {
    $this->pdf = new PDF_Combined($orientation, $unit, $size);
  }
}

load_document_files_from(dirname(dirname(__FILE__)).'/print/docs/racer');

?>