<?php @session_start(); ?>
<?php
require_once('inc/data.inc');
require_once('inc/banner.inc');
require_once('inc/authorize.inc');
require_once('inc/schema_version.inc');
require_once('inc/events.inc');
require_once('inc/lane-bias.inc');

require_permission(CHECK_IN_RACERS_PERMISSION);

?><!DOCTYPE html>
<html>
<head>
<meta http-equiv="Content-Type" content="text/html; charset=UTF-8"/>
<title>Race History</title>
<?php require('inc/stylesheet.inc'); ?>
<script type="text/javascript" src="js/jquery.js"></script>
<script type="text/javascript">
$(function() {
    $("#lane-bias").on('click', function() {
        let details = $("#lane-bias-details");
        if (details.css("display") == "none") {
          // Opening
          $("#lane-bias-triangle").attr('src', 'img/triangle_south.png');
          details.slideDown(200);
        } else {
          // Closing
          $("#lane-bias-triangle").attr('src', 'img/triangle_east.png');
          details.slideUp(200);
        }
      });
  });
</script>
<style type="text/css">
div.content {
  margin-left: 50px;
}

table.event-history {
}

table.event-history tr.event td {
  padding-top: 6px;
  padding-bottom: 6px;
}

table.event-history tr.event-identifier td {
  border-top-width: 2px;
  background: #ffdddd;
}

table.event-history tr.event-explanation td {
  background: #ffdddd;
  border-bottom-width: 2px;
}

table.event-history tr.event td:first-child {
  border-left-width: 2px;
}

table.event-history tr.event td:last-child {
  border-right-width: 2px;
}

#lane-bias-details {
  display: none;
}
</style>
</head>
<body>
<?php
make_banner('Retrospective');
$bias = lane_bias_analysis();
?>
<div class="content">

<h3>Lane Bias Analysis</h3>

  <div id="lane-bias">
<?php
  if (empty($bias)) {
    echo "<p>There isn't enough evidence to assess lane bias.</p>";
  } else {
?>
    <p id="lane-bias-summary">
    <img id="lane-bias-triangle" src="img/triangle_east.png"/>
    &nbsp;
<?php
    if ($bias['biased']) {
      echo "<b>The track lanes appear to be biased, with 90% confidence.</b>";
    } else {
      echo "There is no evidence of significant lane bias.";
    }
?>
    </p>

    <div id="lane-bias-details">

      <table><?php lane_bias_analysis(true); ?></table>

      <?php
        echo "<p>F statistic is ".sprintf("%5.3f", $bias['f-statistic'])." with df1=".$bias['df1']." and df2=".$bias['df2']."</p>\n";
        echo "<p>Critical value for F statistic is ".sprintf("%5.3f", $bias['critical-value'])."</p>\n";
      ?>
</div>
<?php } ?>
</div>

<p>&nbsp;</p>
<h3>Events Timeline</h3>

<table class="event-history">
<?php

$is_interleaved = use_master_sched();

class EventFormatter {
  public function HeatIdentifier(&$event) {
    switch ($event['action']) {
    case EVENT_RESULT_DISCARDED: 
    case EVENT_HEAT_COMPLETED: {
      return $this->HeatName($event);
      break;
    }
    default:
      return "";
    }
  }

  public function Format(&$event) {
    switch ($event['action']) {
    case EVENT_TIMER_MALFUNCTION:
      return 'Timer Malfunction: '.$event['other'];
      break;
    case EVENT_TIMER_RESULT_REFUSED:
      return 'Refused result, lane '.$event['lane'].': '.$event['other'];
    case EVENT_RESULT_DISCARDED: {
      return 'Discarded: lane '.$event['lane']
             .', car '.htmlspecialchars($this->RacerName($event), ENT_QUOTES, 'UTF-8')
      .': '.$event['other'];
      break;
    }
    case EVENT_HEAT_COMPLETED: {
      return 'Original heat completion';
      break;
    }
    case EVENT_HEAT_MANUALLY_ENTERED:
      return 'Manually entered result';
      break;
    default:
      return json_encode($event);
    }
  }

  private function RacerName(&$event) {
      $racer = read_single_row('SELECT carnumber, firstname, lastname'
                               .' FROM RegistrationInfo'
                               .' WHERE racerid = :racerid',
                               array(':racerid' => $event['racerid']));
      return $racer[0].', '.$racer[1].' '.$racer[2];
  }
  
  private function HeatName(&$event) {
    $round = read_single_row('SELECT class, round FROM Rounds'
                             .' INNER JOIN Classes'
                             .' ON Classes.classid = Rounds.classid'
                             .' WHERE Rounds.roundid = :roundid',
                             array(':roundid' => $event['roundid']),
                             PDO::FETCH_ASSOC);
    return $round['class']
        .($round['round'] > 1 ? ' round '.$round['round'] : '')
        .' heat '.$event['heat'];
  }
}

$event_formatter = new EventFormatter;


$event_stmt = $db->prepare('SELECT action, tstamp, roundid, heat, racerid, lane, other'
                           .' FROM Events'
                           .' WHERE action > 10'
                           .' ORDER BY tstamp, lane');
$event_stmt->execute();
$event = $event_stmt->fetch();

$heat_stmt = $db->prepare('SELECT roundid, class, heat, completed'
                          .' FROM RaceChart'
                          .' INNER JOIN Classes ON RaceChart.classid = Classes.classid'
                          .' WHERE COALESCE(completed, \'\') <> \'\''
                          .' GROUP BY roundid, heat'
                          .' ORDER BY completed');
$heat_stmt->execute();
$heat = $heat_stmt->fetch();

$last_unix = 0;  // Unix timestamp of last heat
$roundid = 0;  // Current roundid

while ($event !== false || $heat !== false) {
  if ($heat !== false && ($event === false || $heat['completed'] < $event['tstamp'])) {
    $unix = strtotime($heat['completed']);

    echo "<tr class='heat'>";
    echo "<td>".htmlspecialchars($heat['class'].' heat '.$heat['heat'], ENT_QUOTES, 'UTF-8')."</td>";
    echo "<td>".$heat['completed']."</td>";

    if (($is_interleaved && $roundid != 0) || $heat['roundid'] == $roundid) {
      $diff = $unix - $last_unix;
      $min = floor($diff / 60);
      $sec = $diff % 60;
      echo "<td>".sprintf("%dm%02ds", $min, $sec)."</td>";
    } else {
      echo "<td></td>";
      $roundid = $heat['roundid'];
    }
    $last_unix = $unix;
    echo "</tr>";

    $heat = $heat_stmt->fetch();
  }

  if ($event !== false && ($heat === false || $event['tstamp'] <= $heat['completed'])) {
    echo "<tr class='event event-identifier'>";
    echo "<td>".htmlspecialchars($event_formatter->HeatIdentifier($event), ENT_QUOTES, 'UTF-8')."</td>";
    echo "<td>".$event['tstamp']."</td>";
    echo "<td></td>";
    echo "</tr>\n";
    echo "<tr class='event event-explanation'><td colspan='3'>";

    echo $event_formatter->Format($event);

    echo "<br/>\n";

    echo "</td>";
    echo "</tr>\n";
    $event = $event_stmt->fetch();
  }
}

?>
</table>

</div> <!-- content -->
</body>
</html>
