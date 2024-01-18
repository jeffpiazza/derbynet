<?php @session_start(); ?>
<?php
require_once('inc/data.inc');
require_once('inc/banner.inc');
require_once('inc/authorize.inc');
require_once('inc/schema_version.inc');
require_once('inc/events.inc');
require_once('inc/lane-bias.inc');
require_once('inc/history-formatter.inc');

if (isset($_GET['debug'])) {
  $debug_history_entries = true;
}

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
      echo "<b style='color: red;'>The track lanes appear to be biased, with 90% confidence.</b>";
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
  if (schema_version() < PER_SUBGROUP_AWARDS_SCHEMA) {
    require('inc/old-history.inc');
  } else {
    $formatter = new HistoryActionFormatter;
    $history = $db->prepare('SELECT * FROM ActionHistory ORDER BY received');
    $history->execute();
    foreach ($history as $action) {
      $formatter->SetAction($action['request']);
      $curr_heat = $formatter->CurrentHeatString();
      $step = $formatter->Process();
      if (!empty($step)) {
        $irregular = '';
        echo "<tr class='event $irregular event-identifier'>";
        echo "<td>";
        echo $curr_heat;
        echo "</td>";
        echo "<td>".$action['received']."</td>";
        echo "<td>"./* turnaround time */"</td>";
        echo "</tr>\n";
        echo "<tr class='event $irregular event-explanation'><td colspan='3'>$step</td>";
        echo "</tr>\n";
      }
    }
  }
?>
</table>

</div> <!-- content -->
</body>
</html>
