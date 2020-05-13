<?php @session_start();

require_once('inc/data.inc');
require_once('inc/authorize.inc');
require_once('inc/classes.inc');
require_once('inc/plural.inc');
require_permission(SET_UP_PERMISSION);

require_once('inc/import-csv.inc');

try {
  $classes = all_classes();
} catch (PDOException $p) {
  $classes = [];
}

class ImportRoster extends ImportCsvGenerator {
  protected function make_state_of_play_div() {
    global $classes;

    try {
      $nracers = read_single_value("SELECT COUNT(*) FROM RegistrationInfo", array());
    } catch (PDOException $p) {
      $nracers = -1;
    }
  ?>
    <div id="state-of-play" class="<?php echo $nracers <= 0 ? 'hidden' : ''; ?>">
      <div id="file-stats" class="hidden">
        <span id="file-name">File</span>
        contains <span id="file-racer-count">0</span>
        racers<span id='file-class-count-and-label'>,
        <a id="class-counts-button">
          <span id="file-class-count"></span>
          <?php echo plural(group_label_lc()); ?>
          (<span id='file-class-new-count'></span> new)</a></span>.
      </div>
      <?php
         if ($nracers > 0) {
           $n_classes = count($classes);
           $label = $n_classes == 1 ? group_label_lc() : plural(group_label_lc());
           echo "There are already ".$nracers." racer(s) and ".$n_classes." ".$label
               ." in the database.";
         }
      ?>
   </div><!--- state-of-play -->
  <?php
  }
}
?><!DOCTYPE html>
<html>
<head>
<meta http-equiv="Content-Type" content="text/html; charset=UTF-8"/>
<title>Import Roster</title>
<?php make_head_matter_for_import_page(); ?>
<link rel="stylesheet" type="text/css" href="css/import-roster.css"/>
<script type="text/javascript" src="js/modal.js"></script>
<script type="text/javascript" src="js/plurals.js"></script>
<script type="text/javascript" src="js/import-roster.js"></script>
</head>
<script type="text/javascript">
function all_classes() {
  return <?php echo json_encode($classes, JSON_HEX_TAG | JSON_HEX_AMP | JSON_PRETTY_PRINT); ?>;
}
</script>
<body>
<?php
  make_banner('Import Roster', 'setup.php');
  $page_maker = new ImportRoster;
  $page_maker->make_import_csv_div('Import Roster',
                                   array(
                                     array(
                                       'lastname' => array('name' => "Last Name",
                                                           'required' => true),
                                       'firstname' => array('name' => "First Name",
                                                            'required' => true),
                                       'classname' => array('name' => group_label(),
                                                            'required' => false),
                                       'subgroup' => array('name' => subgroup_label(),
                                                           'required' => false),
                                       'carnumber' => array('name' => "Car Number",
                                                            'required' => false),
                                       'carname' => array('name' => "Car Name",
                                                          'required' => false),
                                       'exclude' => array('name' => 'Exclude?',
                                                          'required' => false)),
                                     array(
                                       'first-last' => array('name' => 'First & Last Name',
                                                             'required' => true,
                                                             'span' => 2),
                                       1 => array('span' => 4)),
                                     ));
?>
<div id="new_ranks_modal" class="modal_dialog block_buttons hidden">
  <div id="existing_ranks">
  </div>
  <div id="new_ranks">
  </div>
  <form>
    <input type="button" data-enhanced="true" value="Dismiss"
      onclick='close_modal("#new_ranks_modal");'/>
  </form>
</div>
<div class="footer">Or instead: <a href="import-results.php">Import results exported from another race...</a></div>
<?php
  require_once('inc/ajax-pending.inc');
?>
</body>
</html>
