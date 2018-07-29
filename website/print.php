<?php
session_start();

require_once('inc/data.inc');
require_once('inc/banner.inc');
require_once('inc/authorize.inc');
require_once('inc/photo-config.inc');

require_once('print/docs/racer/printable_racer_documents.inc');

// TODO Printables should really have their own permission, but we need a
// migration path for updating existing user config files.
require_permission(ASSIGN_RACER_IMAGE_PERMISSION);

$doc_classes = array();
foreach (get_declared_classes() as $c) {
  if (is_subclass_of($c, 'PrintableRacerDocument') && !(new ReflectionClass($c))->isAbstract()) {
    $doc = new $c();
    $doc_classes[$c] = $doc->get_available_options();
  }
}
?><!DOCTYPE html>
<html>
<head>
<meta http-equiv="Content-Type" content="text/html; charset=UTF-8"/>
<title>Printables</title>
<meta name="viewport" content="width=device-width, initial-scale=1"/>
<?php require('inc/stylesheet.inc'); ?>
<link rel="stylesheet" type="text/css" href="css/jquery.mobile-1.4.2.css"/>
<link rel="stylesheet" type="text/css" href="css/print.css"/>
<script type="text/javascript" src="js/jquery.js"></script>
<script type="text/javascript" src="js/jquery-ui-1.10.4.min.js"></script>
<script type="text/javascript" src="js/jquery.ui.touch-punch.min.js"></script>
<script type="text/javascript" src="js/pseudo-mobile.js"></script>
<script type="text/javascript" src="js/print.js"></script>
<script type="text/javascript">
var doc_classes = <?php echo json_encode($doc_classes); ?>;
</script>
</head>
<body>
<?php make_banner('Printables'); ?>
<div class="block_buttons">
    <input type="button" data-enhanced="true" id="print-selected"
           value="Print Selected" onclick="print_selected();"/>
</div>

<div id="page-controls">
<div class="block_buttons">

<p id="sortorder-paragraph"><label for="sortorder">Choose sort order:</label>
   <select id="sortorder" onchange="handle_sortorder_change()">
      <option value="checkin">Check-In Order</option>
      <option value="name">Last Name</option>
      <option value="class"><?php echo htmlspecialchars(group_label(), ENT_QUOTES, 'UTF-8'); ?></option>
      <?php
      if (read_raceinfo_boolean('use-subgroups')) {
        echo "<option value='rank'>".htmlspecialchars(subgroup_label(), ENT_QUOTES, 'UTF-8')."</option>";
      } ?>
      <option value="car" selected="selected">Car Number</option>
    </select>
</p>

    <input type="button" data-enhanced="true" value="Select All" onclick="select_all(true)"/>
    <input type="button" data-enhanced="true" value="Deselect All" onclick="select_all(false)"/>
</div>
</div>

<div id="racers-context">
<div id="racers-scroll">
<div id="racers" class="block_buttons">
<table></table>
</div>
</div>
&nbsp;
</div>

<div id="document-controls">
<?php

$radio_count = 0;
echo "<div data-role='controlgroup'>\n";
foreach ($doc_classes as $c => $options) {
  ++$radio_count;
  $doc = new $c();

  echo "<label for='doc-class-".$c."'>";
  echo "<b>".$doc->name()."</b>";
  echo "</label>\n";

  echo "<input type='radio' name='doc-class' id='doc-class-".$c."'";
  if ($radio_count == 1) {
    echo " checked='checked'";
  }
  echo " value='".$c."'/>";
}
echo "</div>\n";  // controlgroup

foreach ($doc_classes as $c => $options) {
  echo "<div data-docname=\"".$c."\" class=\"sub-options hidden\">";
  $doc = new $c();
  echo "<p>Options for <b>".$doc->name()."</b></p>";
  foreach ($options as $opt => $opt_data) {
    $ctrl_name = $c.'-'.$opt;
    echo "&nbsp;";
    if ($opt_data['type'] == 'bool') {
      echo "<input type='checkbox' name='".$ctrl_name."'";
      if ($opt_data['default']) {
        echo " checked='checked'";
      }
      echo "/><label for='".$ctrl_name."'>".$opt_data['desc']."</label><br/>\n";
    } else if ($opt_data['type'] == 'string') {
      echo "<label for='".$ctrl_name."'>".$opt_data['desc']."</label>";
      echo "<input type='text' name='".$ctrl_name."' value='".$opt_data['default']."'/><br/>\n";
    }
  }
  echo "</div>\n";
}
?>
</div>

</body>
</html>
