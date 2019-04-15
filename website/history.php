<?php @session_start(); ?>
<?php
require_once('inc/data.inc');
require_once('inc/banner.inc');
require_once('inc/authorize.inc');
require_once('inc/schema_version.inc');
require_once('inc/events.inc');
require_permission(CHECK_IN_RACERS_PERMISSION);

?><!DOCTYPE html>
<html>
<head>
<meta http-equiv="Content-Type" content="text/html; charset=UTF-8"/>
<title>Race History</title>
<?php require('inc/stylesheet.inc'); ?>
<script type="text/javascript" src="js/jquery.js"></script>
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

</style>
</head>
<body>
<?php
make_banner('Race History');
?>
<div class="content">
<p>&nbsp;</p>

<table class="event-history">
<?php

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
    case EVENT_RESULT_DISCARDED: {
      return 'Discarded result for lane '.$event['lane']
              .': car '.$this->RacerName($event);
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
      return $racer[0].' '.$racer[1].' '.$racer[2];
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

$heat_stmt = $db->prepare('SELECT roundid, class, heat, completed, strftime(\'%s\', completed) as unix'
                          .' FROM RaceChart'
                          .' INNER JOIN Classes ON RaceChart.classid = Classes.classid'
                          .' GROUP BY roundid, heat'
                          .' ORDER BY unix');
$heat_stmt->execute();
$heat = $heat_stmt->fetch();

$last_unix = 0;  // Unix timestamp of last heat
$roundid = 0;  // Current roundid

while ($event !== false || $heat !== false) {
  if ($heat !== false && ($event === false || $heat['completed'] < $event['tstamp'])) {
    echo "<tr class='heat'>";
    echo "<td>".$heat['class'].' heat '.$heat['heat']."</td>";
    echo "<td>".$heat['completed']."</td>";

    if ($heat['roundid'] == $roundid) {
      $diff = $heat['unix'] - $last_unix;
      $min = floor($diff / 60);
      $sec = $diff % 60;
      echo "<td>".sprintf("%dm%02ds", $min, $sec)."</td>";
    } else {
      echo "<td></td>";
      $roundid = $heat['roundid'];
    }
    $last_unix = $heat['unix'];
    echo "</tr>";

    $heat = $heat_stmt->fetch();
  }

  if ($event !== false && ($heat === false || $event['tstamp'] <= $heat['completed'])) {
    echo "<tr class='event event-identifier'>";
    echo "<td>".$event_formatter->HeatIdentifier($event)."</td>";
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
