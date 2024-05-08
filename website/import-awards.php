<?php @session_start();
require_once('inc/data.inc');
require_once('inc/authorize.inc');
session_write_close();
require_once('inc/partitions.inc');

require_permission(SET_UP_PERMISSION);

require_once('inc/import-csv.inc');

class ImportAwards extends ImportCsvGenerator {
  protected function make_state_of_play_div() {
    try {
      $nawards = read_single_value("SELECT COUNT(*) FROM Awards", array());
    } catch (PDOException $p) {
      $nawards = -1;
    }
    try {
      $ncategories = read_single_value("SELECT DISTINCT(awardtypeid) FROM Awards", array());
    } catch (PDOException $p) {
      $ncategories = -1;
    }
  ?>
    <div id="state-of-play" class="<?php echo $nawards <= 0 ? 'hidden' : ''; ?>">
      <?php
         if ($nawards > 0) {
           echo "There are already ".$nawards." awards(s)"
               .($ncategories > 1 ? " in ".$ncategories." categories" : '')
               ." in the database.";
         }
      ?>
    </div>
  <?php
  }
}

?><!DOCTYPE html>
<html>
<head>
<meta http-equiv="Content-Type" content="text/html; charset=UTF-8"/>
<title>Import Awards</title>
<?php make_head_matter_for_import_page(); ?>
<script type="text/javascript" src="js/import-awards.js"></script>
</head>
<body>
<?php
  make_banner('Import Awards', 'setup.php');
  $page_maker = new ImportAwards;
  $page_maker->make_import_csv_div('Import Awards',
                                   array(
                                     'awardname' => array('name' => "Award Name",
                                                          'required' => true),
                                     'awardtype' => array('name' => "Award Type",
                                                          'required' => true),
                                     'classname' => array('name' => group_label(),
                                                          'required' => false),
                                     'subgroup' => array('name' => subgroup_label(),
                                                         'required' => false),
                                     'carnumber' => array('name' => "Winning Car Number",
                                                          'required' => false)));
  require_once('inc/ajax-pending.inc');
?>
</body>
</html>
