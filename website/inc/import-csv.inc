<?php
require_once('inc/banner.inc');

function make_head_matter_for_import_page() {
  ?>
    <link rel="stylesheet" type="text/css" href="css/jquery.mobile-1.4.2.css"/>
    <?php require('inc/stylesheet.inc'); ?>
    <link rel="stylesheet" type="text/css" href="css/import-csv.css"/>
    <script type="text/javascript" src="js/jquery.js"></script>
    <script type="text/javascript" src="js/mobile-init.js"></script>
    <script type="text/javascript" src="js/jquery-ui-1.10.4.min.js"></script>
    <script type="text/javascript" src="js/jquery.ui.touch-punch.min.js"></script>
    <script type="text/javascript" src="js/jquery.mobile-1.4.2.min.js"></script>
    <script type="text/javascript" src="js/dashboard-ajax.js"></script>
    <script type="text/javascript" src="js/checkin.js"></script>
    <script type="text/javascript" src="js/jquery.csv.js"></script>
    <script type="text/javascript" src="js/import-csv.js"></script>
<?php
}



class ImportCsvGenerator {
  // Override as needed
  protected function make_state_of_play_div() {}
  
  // Override to null if no encoding section to be shown.
  protected function make_encoding_section() {
?>

  <div id="encoding">
    <div id="encoding-guidance">

      <p>The choice of encoding primarily affects the treatment of accented
      or other &quot;special&quot; characters.  Comma-separated value (CSV) files produced
      by Microsoft Excel typically use Windows or Macintosh encoding, depending
      on platform.</p>

      <p>If you have a CSV file that contains only ASCII characters, then it won&#39;t
      matter which of these encodings you choose.  Also, trial and error is a
      perfectly acceptable method of figuring out what encoding renders your file
      correctly.</p>

    </div>

    <div class="encoding-div">
      <label for="encoding-select" id="encoding_message">
      Please select encoding (BEFORE selecting file to import):
      </label>
      <br/>    <input type="radio" name="encoding-select" data-enhanced="true" checked="checked"
                      value="utf-8">UTF-8</input>
      <br/>    <input type="radio" name="encoding-select" data-enhanced="true"
                      value="macintosh">Macintosh</input>
      <br/>    <input type="radio" name="encoding-select" data-enhanced="true"
                      value="cp1252">Windows (cp1252)</input>
      <br/>    <input type="radio" name="encoding-select" data-enhanced="true"
                      value="cp437">MS-DOS (cp437)</input>
    </div>
  </div>
<?php
  }

  protected function make_import_button($button_label) {
?>
  <div id="import_button_div" class="block_buttons hidden">
    <input type="button" id="import_button" data-enhanced="true"
           value="<?php echo htmlspecialchars($button_label, ENT_QUOTES, 'UTF-8'); ?>"/>
  </div>
<?php
    }

  // $labels is an array of field => {name:, required:, (span:)}
  //
  // 'field' is normally a string (the key to be sent), but may be an integer to
  // mark a blank cell.  'name' is the name to be shown in the UI.
  //
  // Alternatively, $labels can be an array of such arrays, in which case more
  // than one label row will be produced.  In this case, span: attributes tell how
  // many table columns the cell should span.
  protected function make_column_labels($labels) {
    if (count($labels) == 0) {
      return;
    }
    if (!isset($labels[0]) || is_string(@$labels[0]['name'])) {
      $labels = array($labels);
    }
?>
  <div class="fields hidden">
    <h3>Drag labels to data columns:</h3>

    <div class="target"> <!-- targets for column labels -->
      <table id="labels_table">
       <?php foreach ($labels as $row) { ?>
        <tr>
           <?php
             foreach ($row as $field => $details) {
               echo '<td data-home="'.$field.'"'.(isset($details['span']) ? ' colspan="'.$details['span'].'"' : '').'>';
               if (is_string($field)) {
                 echo '<div data-field="'.$field.'" class="field '
                     .($details['required'] ? 'required' : 'optional').'">'
                     .htmlspecialchars($details['name'], ENT_QUOTES, 'UTF-8')
                     .'</div>';
               }
               echo "</td>\n";
             }
           ?>
        </tr>
       <?php } ?>
      </table>
    </div><!-- target -->
  </div>
<?php
  }

  public function make_import_csv_div($button_label, $labels) {
    // The #file_target div's <input> is invisible (opacity 0) and z-index'ed on
    // top of the entire #file_target.  Drag and drop behavior is added by jquery-ui.
?>
<div class="import_csv">
  <div id="top_matter">
    <?php $this->make_state_of_play_div(); ?>

    <?php $this->make_encoding_section(); ?>
    <div id="controls">
      <?php $this->make_import_button($button_label); ?>
      <div class="hidden" id="assign_label_message">Assign field labels to enable file import.</div>
    </div><!-- controls -->

    <div class="file_target">
      <input type="file" id="csv_file" name="csv_file" data-enhanced="true"/>
      <label for="csv_file">
      <div id="drop_img"><img src="img/drop.png"/></div>
      <div id="please_select_message">
         Please select a file to import<br/>or drag it here.
     </div>
      </label>
    </div>

  </div><!-- top_matter -->

  <?php $this->make_column_labels($labels); ?>

  <table id="csv_content">
  </table>

</div><!-- import_csv -->
<?php
    }
}
?>