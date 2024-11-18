<?php
require_once('print/inc/layout.inc');
require_once('print/inc/load_document_files.inc');

abstract class PrintableRacerDocument {
  protected $pdf;
  protected $layout;
  protected $options;  // Contents vary by document type

  protected $ok = true;

  // Returns an array of key => {type:, desc:, default:}, with an entry for each
  // option key understood by the document class.
  //
  // type can be one of 'bool', 'string', 'int', or 'radio'.
  //
  // If 'radio', then there's an additional field present, values:, containing
  // an array of {value:, desc:}.  The value of the selected option is returned
  // as a string.
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
  // imagefile_fullrez
  // imagefile  // smaller rez if available
  // carphoto_fullrez
  // carphoto  // smaller rez if available
  // barcode
  abstract function DrawOneAt($left, $top, &$racer);

  public function DrawOne(&$racer) {
    if ($this->ok) {
      $xy = $this->layout->NextTopLeft();
      try {
        $this->DrawOneAt($xy['x'], $xy['y'], $racer);
      } catch (Exception $ex) {
        $this->ok = false;
        echo "<p class='error'>An exception occurred in processing for <b>"
            .htmlspecialchars($racer['firstname']." ".$racer['lastname'], ENT_QUOTES, 'UTF-8')
            ." ($racer[carnumber])</b>:";
        while ($ex) {
          echo "<br/>";
          echo "&nbsp; <b>".$ex->getMessage()."</b>";
          $ex = $ex->getPrevious();
        }
        echo "</p>";
      }
    }
  }

  public function Output() {
    if ($this->ok) {
      $this->pdf->Output();
    }
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
