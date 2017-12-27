<?php session_start(); ?>
<?php
require_once('inc/data.inc');
require_once('inc/banner.inc');
require_once('inc/authorize.inc');
require_permission(PRESENT_AWARDS_PERMISSION);
?><!DOCTYPE html>
<html>
<head>
<meta http-equiv="Content-Type" content="text/html; charset=UTF-8"/>
<title>Kiosk Dashboard</title>
<?php require('inc/stylesheet.inc'); ?>
<link rel="stylesheet" type="text/css" href="css/jquery.mobile-1.4.2.css"/>
<script type="text/javascript" src="js/jquery.js"></script>
<script type="text/javascript" src="js/jquery-ui-1.10.4.min.js"></script>
<link rel="stylesheet" type="text/css" href="css/kiosk-dashboard.css"/>
<script type="text/javascript" src="js/mobile-init.js"></script>
<script type="text/javascript" src="js/jquery.mobile-1.4.2.min.js"></script>
<script type="text/javascript" src="js/dashboard-ajax.js"></script>
<script type="text/javascript" src="js/modal.js"></script>
<script type="text/javascript" src="js/kiosk-dashboard.js"></script>
</head>
<body>
<?php make_banner('Kiosk Dashboard'); ?>
<?php require_once('inc/ajax-failure.inc'); ?>

<div class="standings-control hidden control_group block_buttons">
  <div class="round-select">
    <h3>Display standings for:</h3>
    <select>
      <?php
        // This <select> elements lets the operator choose what standings should be displayed on
        // kiosks displaying standings.
        $current = explode('-', read_raceinfo('standings-message'));
        $current_roundid = $current[0];
        $current_rankid = $current[1];

        require_once('inc/standings.inc');
        $use_subgroups = read_raceinfo_boolean('use-subgroups');

        $sel = ' selected="selected"';
        if (count($current) == 0) {
          echo '<option '.$sel.' disabled="1">Please choose what standings to display</option>';
        }
        echo '<option data-roundid=""'.((count($current) != 0 && $current_roundid == '') ? $sel : '').'>'
             .supergroup_label()
             .'</option>';
        foreach (rounds_for_standings() as $round) {
          echo '<option data-roundid="'.$round['roundid'].'" data-rankid=""'
               .($current_roundid == $round['roundid'] && $current_rankid == '' ? $sel : '').'>'
               .htmlspecialchars($round['name'], ENT_QUOTES, 'UTF-8')
               .'</option>'."\n";
          if ($use_subgroups) {
            foreach ($round['ranks'] as $rank) {
              echo '<option data-roundid="'.$round['roundid'].'" data-rankid="'.$rank['rankid'].'"'
              .($current_roundid == $round['roundid'] && $current_rankid == $rank['rankid'] ? $sel : '').'>';
              echo htmlspecialchars($round['name'].' / '.$rank['name'], ENT_QUOTES, 'UTF-8');
              echo "</option>\n";
            }
          }
        }
      ?>
    </select>
  </div>
  <div class="reveal block_buttons">
    <input type="button" data-enhanced="true" value="Reveal One" onclick="handle_reveal1()"/><br/>
    <input type="button" data-enhanced="true" value="Reveal All" onclick="handle_reveal_all()"/><br/>
  </div>
</div><!-- standings-control -->

<div id="kiosk_control_group" class="kiosk_control_group">
</div>


<div id='kiosk_modal' class="modal_dialog hidden block_buttons">
  <form>
    <label for="kiosk_name_field">Name for kiosk:</label>
    <input type="text" id="kiosk_name_field"/>
    <input type="submit" data-enhanced="true" value="Assign"/>
    <input type="button" data-enhanced="true" value="Cancel"
      onclick='close_modal("#kiosk_modal");'/>
  </form>
</div>

<div id='config_classes_modal' class="modal_dialog hidden block_buttons">
  <form>
    <?php
        require_once('inc/schema_version.inc');

        $stmt = $db->prepare('SELECT classid, class'
                             .' FROM Classes'
                             .' WHERE EXISTS(SELECT 1 FROM RegistrationInfo'
                             .'              WHERE RegistrationInfo.classid = Classes.classid)'
                             .' ORDER BY '.(schema_version() >= 2
                                            ? 'sortorder, ' : '')
                             .'class');
        $stmt->execute(array());

        foreach ($stmt as $row) {
          echo '<input type="checkbox" name="class-'.$row['classid'].'"'
               .' id="config-class-'.$row['classid'].'"'
               .' data-classid="'.$row['classid'].'"'
               .'/>'."\n";
          echo '<label for="config-class-'.$row['classid'].'">'
              .htmlspecialchars($row['class'], ENT_QUOTES, 'UTF-8')
              .'</label>'."\n";
        }
    ?>
    <input type="submit" data-enhanced="true" value="Configure Kiosk"/>
    <input type="button" data-enhanced="true" value="Cancel"
      onclick='close_modal("#config_classes_modal");'/>
  </form>
</div>

</body>
</html>