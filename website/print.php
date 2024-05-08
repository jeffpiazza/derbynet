<?php
session_start();

require_once('inc/data.inc');
require_once('inc/authorize.inc');
session_write_close();
require_once('inc/partitions.inc');
require_once('inc/banner.inc');
require_once('inc/photo-config.inc');

require_once('print/inc/printable_racer_document.inc');
require_once('print/inc/printable_award_document.inc');
require_once('print/inc/printable_summary_document.inc');

// TODO Printables should really have their own permission, but we need a
// migration path for updating existing user config files.
require_permission(ASSIGN_RACER_IMAGE_PERMISSION);

// $doc_classes keys are document class names, values are { type:, options: }
$doc_classes = array();
foreach (get_declared_classes() as $c) {
  if (is_subclass_of($c, 'PrintableRacerDocument') && !(new ReflectionClass($c))->isAbstract()) {
    $doc = new $c();
    $doc_classes[$c] = array('type' => 'racer',
                             'order' => 1,
                             'name' => $doc->name(),
                             'options' => $doc->get_available_options());
  }
  if (is_subclass_of($c, 'PrintableAwardDocument') && !(new ReflectionClass($c))->isAbstract()) {
    $doc = new $c();
    $doc_classes[$c] = array('type' => 'award',
                             'order' => 2,
                             'name' => $doc->name(),
                             'options' => $doc->get_available_options());
  }
  if (is_subclass_of($c, 'PrintableSummaryDocument') && !(new ReflectionClass($c))->isAbstract()) {
    $doc = new $c();
    $doc_classes[$c] = array('type' => 'summary',
                             'order' => 3,
                             'name' => $doc->name(),
                             'options' => $doc->get_available_options());
  }
}

function order_by_name($left, $right) {
  if ($left['order'] < $right['order']) {
    return -1;
  } else if ($left['order'] > $right['order']) {
    return 1;
  } else  if ($left['name'] < $right['name']) {
    return -1;
  } else if ($left['name'] > $right['name']) {
    return 1;
  } else {
    return 0;
  }
}

uasort($doc_classes, 'order_by_name');

?><!DOCTYPE html>
<html>
<head>
<meta http-equiv="Content-Type" content="text/html; charset=UTF-8"/>
<title>Printables</title>
<meta name="viewport" content="width=device-width, initial-scale=1"/>
<link rel="stylesheet" type="text/css" href="css/jquery-ui.min.css"/>
<?php require('inc/stylesheet.inc'); ?>
<link rel="stylesheet" type="text/css" href="css/mobile.css"/>
<link rel="stylesheet" type="text/css" href="css/print.css"/>
<script type="text/javascript" src="js/jquery.js"></script>
<script type="text/javascript" src="js/ajax-setup.js"></script>
<script type="text/javascript" src="js/jquery-ui.min.js"></script>
<script type="text/javascript" src="js/jquery.ui.touch-punch.min.js"></script>
<script type="text/javascript" src="js/mobile.js"></script>
<script type="text/javascript" src="js/print.js"></script>
<script type="text/javascript">
var doc_classes = <?php echo json_encode($doc_classes); ?>;
</script>
</head>
<body>
<?php make_banner('Printables'); ?>


<div class="block_buttons">
    <input type="button" id="print-selected"
           value="Print Selected" onclick="print_selected();"/>
</div>


<div id="document-controls">
<?php

$radio_count = 0;
$last_type = '';
echo "<div class='mradiogroup'>\n";
foreach ($doc_classes as $c => $details) {
  ++$radio_count;
  $options = $details['options'];

  if ($radio_count > 0 && $details['type'] != $last_type) {
    echo "<div class='radio-spacer'>&nbsp;</div>\n";
  }
  $last_type = $details['type'];

  echo "<label for='doc-class-".$c."'>";
  echo "<b>".$details['name']."</b>";
  echo "</label>\n";

  echo "<input type='radio' name='doc-class' id='doc-class-".$c."'";
  if ($radio_count == 1) {
    echo " checked='checked'";
  }
  echo " value='".$c."'/>";
}
echo "</div>\n";  // controlgroup

// Options for each document type are in a div[data-docname='...'], and so can
// be switched on and off depending on which document type is chosen.
$doc_index = 0;  // To distinguish radio options, if needed
foreach ($doc_classes as $c => $details) {
  ++$doc_index;
  echo "<div data-docname=\"".$c."\" class=\"sub-options hidden\">";
  echo "<p>Options for <b>".$details['name']."</b></p>";
  foreach ($details['options'] as $opt => $opt_data) {
    $ctrl_name = $c.'-'.$opt;
    echo "<div class='param'>\n";
    if ($opt_data['type'] == 'bool') {
      echo "<input type='checkbox' name='".$ctrl_name."'";
      if ($opt_data['default']) {
        echo " checked='checked'";
      }
      echo "/><label for='".$ctrl_name."'>".$opt_data['desc']."</label><br/>\n";
    } else if ($opt_data['type'] == 'int') {
      echo "<input type='number' name='".$ctrl_name."' value='".$opt_data['default']."'/>";
      echo "<label for='".$ctrl_name."'>".$opt_data['desc']."</label><br/>\n";
    } else if ($opt_data['type'] == 'string') {
      echo "<label for='".$ctrl_name."'>".$opt_data['desc']."</label>";
      echo "<input type='text' name='".$ctrl_name."' value='".$opt_data['default']."' class='param-string'/><br/>\n";
    } else if ($opt_data['type'] == 'radio') {
      // values:
      $first_radio = true;
      echo "<div class='mradiogroup'>\n";
      foreach ($opt_data['values'] as $v) {
        // {value:, desc:}
        echo "<input type='radio' id=\"opt-$doc_index-$opt-$v[value]\""
                  .($first_radio ? " checked=\"checked\"" : "")
                  ." name=\"$ctrl_name\" value=\"$v[value]\"/>\n";
        echo "<label for=\"opt-$doc_index-$opt-$v[value]\">$v[desc]</label>\n";
        $first_radio = false;
      }
      echo "</div>\n";
    }
    echo "</div>\n";
  }
  echo "</div>\n";
}
?>
</div>


<div id="page-controls">
<div class="block_buttons">

   <div id="sortorder-racers-div">
     <p id="sortorder-paragraph">
       <label for="sortorder-racers">Choose sort order:</label>

       <select id="sortorder-racers" onchange="handle_sortorder_racers_change()">
         <option value="checkin">Check-In Order</option>
          <option value="name">Last Name</option>
          <option value="class"><?php echo htmlspecialchars(group_label(), ENT_QUOTES, 'UTF-8'); ?></option>
      <?php
      if (use_subgroups()) {
        echo "<option value='rank'>".htmlspecialchars(subgroup_label(), ENT_QUOTES, 'UTF-8')."</option>";
      } ?>
          <option value="car" selected="selected">Car Number</option>
        </select>
      </p>
    </div>

    <div id="sortorder-awards-div">
    </div>

    <div id="sortorder-summary-div">
    </div>

    <input type="button" value="Select All" onclick="select_all(true)"/>
    <input type="button" value="Deselect All" onclick="select_all(false)"/>
</div>
</div>



<div id="subjects-context">
  <div id="subjects-scroll">
    <div id="subject-racers" class="block_buttons">
    <?php
        if (read_raceinfo_boolean('fake-racers')) {
          echo "<h3 style='text-align: center; color: red;'>Photos for fake racers<br/>do not render in printables</h3>\n";
        }
      ?>
      <table></table>
    </div>
    <div id="subject-awards" class="block_buttons hidden">
      <table></table>
    </div>
    <div id="subject-summary" class="block_buttons hidden">
      <table></table>
    </div>
  </div>
  &nbsp;
</div>

</body>
</html>
