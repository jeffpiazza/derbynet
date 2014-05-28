<?php session_start(); ?>
<?php
require_once('inc/data.inc');
require_once('inc/authorize.inc');
require_permission(SET_UP_PERMISSION);  // TODO: What's the correct permission?
require_once('inc/timer-state.inc');
?>
<html>
<head>
<title>Race Coordinator Page</title>
<?php require('inc/stylesheet.inc'); ?>
<style type="text/css">
.control_column {
  display: inline-block;
  vertical-align: top;
  width: 500px;
}

.control_group {
  border: 3px solid #023882;
  margin: 10px;
  padding: 10px;
 }
</style>
</head>
<body>
<?php
$banner_title = 'Race Coordinator Page';
require('inc/banner.inc');

// TODO: Cut-and-pasted; remove from settings.php
function scan_kiosks($prefix, $kiosk_page) {
    $dh = opendir(dirname(__FILE__).DIRECTORY_SEPARATOR.$prefix);
    while (($entry = readdir($dh)) !== FALSE) {
        if (substr($entry, -6) == ".kiosk") {
            echo '<option '.($entry == $kiosk_page ? 'selected="1" ' : '')
            .'value="'.$prefix.DIRECTORY_SEPARATOR.$entry.'">'.$entry.'</option>'."\n";
        }
    }
    closedir($dh);
}

?>
<div class="control_column">

<div class="kiosk_control_group">
<div class="block_buttons">
<?php
try {
    $stmt = $db->query('SELECT address, name, page, last_contact FROM Kiosks ORDER BY name, last_contact');
    foreach ($stmt as $row) {
        echo '<div class="control_group kiosk_control">'."\n";
        echo   '<p>Kiosk ';
        echo   '<span class="kiosk_control_name">'.htmlspecialchars($row['name']).'</span> ';
        echo   '<span class="kiosk_control_address">'.htmlspecialchars($row['address']).'</span> ';
        // TODO: last_contact
        echo   '</p>';

        echo   '<label for="kiosk-page">Display:</label>'; // TODO: Fire action when selection changes.
        echo   '<select name="kiosk-page" data-kiosk-address="'.htmlspecialchars($row['address']).'">'."\n";
        echo   '<optgroup>'."\n";

        scan_kiosks('kiosks', $row['page']);
        scan_kiosks('local'.DIRECTORY_SEPARATOR.'kiosks', $row['page']);

        echo   '</optgroup>'."\n";
        echo   '</select>'."\n";
        echo '</div>'."\n";
    }
} catch (PDOException $p) {
    // TODO: No Kiosks table
    echo '<h2>No Kiosks table defined.</h2>';  // TODO: Or other problem...
    // TODO: What if not throwing exceptions?
}
?>
</div>
</div>

<div class="control_group timer_control_group">
  <!-- TODO: Show timer status -->
  <h3>Timer Status</h3>
  <p><b><?php
    $tstate = get_timer_state();
    if ($tstate == TIMER_NOT_CONNECTED) {
        echo 'NOT CONNECTED';
    } else if ($tstate == TIMER_CONNECTED) {
        echo 'CONNECTED ('.(time() - read_raceinfo('timer_last_contact')).' sec.)';
    } else if ($tstate == TIMER_PREPARED) {
        echo 'Prepared';
    } else if ($tstate == TIMER_RUNNING) {
        echo 'Race running';
    } else {
        echo 'Unknown ('.$tstate.')';
    }
?></b></p>
  <p>The track has <?php echo get_lane_count(); ?> lane(s).</p>
  <!-- TODO: Timer events:
  Timer to database:
      Hello/identity (type of timer, number of lanes)
      Heartbeat
      Gate opened
      Race finished
  DB to timer:
      Prepare timer (with lane mask)
      Number of lanes (configured by race coordinator)
      Heat cancel/abort
  -->
</div>

<div class="control_group replay_control_group">
  <!-- TODO: Show instant-replay status -->
  <h3>Instant Replay Status</h3>
</div>

</div>

<div class="control_column">

<div class="control_group heat_control_group">
  <!-- TODO: Next heat/previous heat, manual heat results -->
  <h3>Heat Control</h3>
</div>

<div class="scheduling_control_group">
<div class="block_buttons">
<?php

$curr_round = get_running_round();
// TODO: Control of ordering of rounds

// TODO: Create 2nd round, grand finals round, including roster

$stmt = $db->query('SELECT roundid, Classes.class, round FROM Rounds'
                   .' INNER JOIN Classes ON Rounds.classid = Classes.classid'
                   .' ORDER BY round, Classes.class');
foreach ($stmt as $round) {
    echo '<div class="control_group scheduling_control">'."\n";
    echo '<p>'.htmlspecialchars($round['class']).', round '.$round['round'].'</p>'."\n";
    // TODO: Actions for these buttons
    // TODO: Buttons enable/disable according to state of the round.
    // TODO: States for a round: Not scheduled; scheduled but not raced; partial results; completed
    // TODO: Count racers in a round/roster, count passed racers in a round
    echo '<input type="button" value="Schedule"/>'."\n";
    echo '<input type="button" value="Reschedule"/>'."\n";
    echo '<input type="button" value="Race"/>'."\n";
    echo '<input type="button" value="Discard Results"/>'."\n";
    if (@$curr_round['roundid'] == $round['roundid']) {
        // TODO: Master schedule?
        echo '<p>(Currently racing)</p>';
    }
    echo '</div>'."\n";
}
?>
</div>
</div>
</div>

</body>
</html>