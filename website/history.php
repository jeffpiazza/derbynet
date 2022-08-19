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
}

table.event-history tr.event-explanation td {
  border-bottom-width: 2px;
}

table.event-history tr.event.irregular td {
  background: #ffdddd;
}

table.event-history tr.event-explanation.discarded-event td {
  text-align: left;
}
table.event-history tr.event-explanation.first td {
  border-bottom-width: 0px;
}
table.event-history tr.event-explanation.follow-on td {
  border-top-width: 0px;
}
/* first, follow-on */

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
    case EVENT_HEAT_RESULT_DISCARDED: 
    case EVENT_HEAT_RESULT_FROM_TIMER:
    case EVENT_HEAT_RESULT_DISCARDED:
    case EVENT_HEAT_RESULT_REINSTATED:
    {
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
      return "Refused result, lane $event[lane]: $event[other]";
    case EVENT_TIMER_OVERDUE:
      return "Timer heartbeat transmission overdue by $event[other]ms";
    case EVENT_HEAT_RESULT_DISCARDED:
      return "Heat results discarded";
    case EVENT_HEAT_RESULT_REINSTATED:
      return "Heat results reinstated";
    case EVENT_HEAT_RESULT_FROM_TIMER: {
      return 'Original heat completion';
    }
    case EVENT_HEAT_MANUALLY_ENTERED:
      return 'Manually entered result';

    case EVENT_CLASS_ADDED:
      return 'Created group '.htmlspecialchars($event['other'], ENT_QUOTES, 'UTF-8')
          ." (#$event[classid])";
    case EVENT_CLASS_DELETED:
      return "Deleted group $event[other] ($event[classid])";
    case EVENT_RANK_ADDED:
      return 'Created subgroup '.htmlspecialchars($event['other'], ENT_QUOTES, 'UTF-8')
      .' (#'.$event['rankid'].') in '.$this->ClassName($event);
    case EVENT_RANK_DELETED:
      if ($event['classid'] < 0) {
        return "Deleted all subgroups from $event[other] (#$event[classid])";
      } else {
        return "Deleted subgroup $event[other] (#$event[rankid])";
      }
    case EVENT_ROUND_ADDED:
      return "Created $event[other] (#$event[roundid]) for ".$this->ClassName($event);
    case EVENT_ROUND_DELETED:
      return "Deleted round $event[roundid] for ".$this->ClassName($event);
    case EVENT_SCHEDULE_ADDED:
      return "Generated schedule for ".$this->RoundName($event);
    case EVENT_SCHEDULE_DELETED:
      return "Deleted schedule for ".$this->RoundName($event)." $event[other]";
    case EVENT_PURGE_RESULTS:
      return "Purged race results";
    case EVENT_PURGE_RESULTS_ONE_ROUND:
      return "Purge race results for ".$this->RoundName($event);
    case EVENT_PURGE_SCHEDULES:
      return "Purged schedules";
    case EVENT_PURGE_RACERS:
      return "Purged racers";
    case EVENT_PURGE_AWARDS:
      return "Purged awards";
      
    case EVENT_ROUND_DELETED:
      return "Deleted round $event[roundid]: $event[other]";

    case EVENT_GROUP_FORMATION_RULE_APPLIED:
      return "Changed group formation rule, $event[other]";
    case EVENT_PARTITION_MOVED:
      return "Moved partition $event[other]";
    case EVENT_PARTITION_RENAMED:
      return "Renamed partition $event[other]";

    default:
      return json_encode($event);
    }
  }

  private function RacerName(&$event) {
    $racer = read_single_row('SELECT carnumber, firstname, lastname'
                             .' FROM RegistrationInfo'
                             .' WHERE racerid = :racerid',
                             array(':racerid' => $event['racerid']));
    return htmlspecialchars("$racer[0], $racer[1] $racer[2] (#$event[racerid])",
                            ENT_QUOTES, 'UTF-8');
  }
  
  private function HeatName(&$event) {
    return $this->RoundName($event)." heat $event[heat]";
  }

  private function RoundName(&$event) {
    $round = read_single_row('SELECT class, round FROM Rounds'
                             .' INNER JOIN Classes'
                             .' ON Classes.classid = Rounds.classid'
                             .' WHERE Rounds.roundid = :roundid',
                             array(':roundid' => $event['roundid']),
                             PDO::FETCH_ASSOC);
    if ($round === false) {
      return htmlspecialchars("roundid =$event[roundid]", ENT_QUOTES, 'UTF-8');
    } else if ($round['round'] > 1) {
      return htmlspecialchars("$round[class] round $round[round] (#$event[roundid])", ENT_QUOTES, 'UTF-8');
    } else {
      return htmlspecialchars("$round[class] (R#$event[roundid])", ENT_QUOTES, 'UTF-8');
    }
  }

  private function ClassName(&$event) {
    $c = read_single_value('SELECT class FROM Classes WHERE classid = :classid',
                           array(':classid' => $event['classid']));
    return htmlspecialchars("$c (#$event[classid])", ENT_QUOTES, 'UTF-8');
  }
}

$event_formatter = new EventFormatter;


$event_stmt = $db->prepare('SELECT tstamp, action, racerid, classid, rankid, roundid, heat, other'
                           .' FROM Events'
                           .' WHERE action > 10'
                           .' ORDER BY tstamp');
$event_stmt->execute();

foreach ($event_stmt as $event) {

// Each heat is <heat identifier>  <time and date>  <delta-t>
//
// Each event is two rows:
//  <heat identifier>  <timestamp>   <empty>
//    <event description>

    $irregular = $event['action'] == EVENT_HEAT_RESULT_DISCARDED ||
               $event['action'] == EVENT_HEAT_RESULT_REINSTATED ||
               $event['action'] >= 300
               ? 'irregular' : '';
    echo "<tr class='event $irregular event-identifier'>";
    echo "<td>".$event_formatter->HeatIdentifier($event)."</td>";
    echo "<td>".$event['tstamp']."</td>";
    echo "<td></td>";
    echo "</tr>\n";
    echo "<tr class='event $irregular event-explanation'><td colspan='3'>";
    echo $event_formatter->Format($event);
    echo "</td>";
    echo "</tr>\n";

}

?>
</table>

</div> <!-- content -->
</body>
</html>
