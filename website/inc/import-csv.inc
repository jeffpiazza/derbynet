<?php

function generate_import_page_head_matter() {
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

function generate_import_page_encoding_section() {
?>
  <div id="import_button_div">
    <input class="hidden" type="button" id="import_button" data-enhanced="true" value="Import Roster"/>
  </div>

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

// $labels is an array of field => {name:, required:}
function generate_import_page_column_labels(&$labels) {
?>
  <div class="fields hidden">
    <h3>Drag fields to label the data columns for import.</h3>

    <div class="target"> <!-- targets for column labels -->
      <table>
        <tr>
          <?php
             foreach ($labels as $field => $details) {
               echo '<td data-home="'.$field.'">';
               echo '<div data-field="'.$field.'" class="field '
                 .($details['required'] ? 'required' : 'optional').'">'.$details['name'].'</div>';
               echo "</td>\n";
             }
          ?>
        </tr>
      </table>
    </div><!-- target -->
  </div>
<?php
}


function generate_import_page($page_name, $script_files, $show_encodings, $labels, $footer) {
  try {
    $racers = read_single_value("SELECT COUNT(*) FROM RegistrationInfo", array());
  } catch (PDOException $p) {
    $racers = -1;
  }
?><!DOCTYPE html>
<html>
<head>
<meta http-equiv="Content-Type" content="text/html; charset=UTF-8"/>
<title><?php echo $page_name; ?></title>
<?php
  generate_import_page_head_matter();
  foreach ($script_files as $script) {
      echo '<script type="text/javascript" src="'.$script.'"></script>'."\n";
  }
?>
</head>
<body>
<?php $banner_title = $page_name; require('inc/banner.inc'); ?>
<div class="import_csv">
  <div id="top_matter">
    <div id="new_ranks">
    </div>

    <div id="controls">
      <div id="meta">
      </div>

      <?php if ($racers > 0) { ?>
        <p>There are already <?php echo $racers; ?> racer(s) registered.
        Re-initialize the database schema if you&#39;re trying to start fresh.</p>
      <?php } else if ($racers < 0) { ?>
        <p>The RegistrationInfo table could not be read: you should probably
        finish setting up the database.</p>
      <?php } ?>

      <form method="link" action="database-setup.php">
        <input type="submit" data-enhanced="true" value="Set Up Database"/>
      </form>

      <form method="link">
        <input type="submit" id="start_over_button" data-enhanced="true"
               class="hidden" value="Start Over"/>
      </form>

      <?php if ($show_encodings) { generate_import_page_encoding_section(); } ?>

      <div id="submit_message">Please select or drag a file to import.
      </div>

    </div><!-- controls -->

    <div class="file_target">
      <input type="file" id="csv_file" name="csv_file"/>
    </div>

  </div><!-- top_matter -->

  <?php if (count($labels) > 0) { generate_import_page_column_labels($labels); } ?>

  <table id="csv_content">
  </table>

</div><!-- import_csv -->
<?php
  echo $footer;
  require_once('inc/ajax-pending.inc');
?>
</body>
</html>
<?php
}
?>