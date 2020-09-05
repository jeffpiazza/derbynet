<?php @session_start();

// $_GET['back'] if "Back" button should go to another page, otherwise coordinator.php.

require_once('inc/data.inc');
require_once('inc/banner.inc');
require_once('inc/schema_version.inc');
require_once('inc/authorize.inc');
require_once('inc/kiosks.inc');
require_once('inc/scenes.inc');
require_once('inc/classes.inc');
require_once('inc/racing-queue.inc');
require_once('inc/rounds.inc');

require_permission(SET_UP_PERMISSION);

list($classes, $classeq, $ranks, $rankseq) = classes_and_ranks();
$all_scenes = array();
foreach (all_scenes() as $sc) {
  $all_scenes[$sc['sceneid']] = $sc;
}
$use_groups = use_groups();
?><!DOCTYPE html>
<html>
<head>
<title>Racing Queue</title>
<?php require('inc/stylesheet.inc'); ?>
<link rel="stylesheet" type="text/css" href="css/jquery-ui.min.css"/>
<link rel="stylesheet" type="text/css" href="css/mobile.css"/>
<script type="text/javascript" src="js/jquery.js"></script>
<script type="text/javascript" src="js/jquery-ui.min.js"></script>
<script type="text/javascript" src="js/jquery.ui.touch-punch.min.js"></script>
<script type="text/javascript" src="js/mobile.js"></script>
<script type='text/javascript' src="js/modal.js"></script>
<style type='text/css'>
#main {
  position: relative;
  width: 800px;
  margin-left: auto;
  margin-right: auto;
}

#queue {
  width: 400px;
}

#racing-scene-div {
  margin-bottom: 30px;
}

#queue ul {
padding-bottom: 50px;
}
#queue ul.mlistview > li {
border-width: 0px;
padding: 0em 1em;
}

#queue ul.mlistview > li > div.queued-round {
border: 3px black solid;
margin: 3px;
margin-bottom: 0px;
margin-top: 0px;
}

#queue ul.mlistview > li input.queue-remove {
float: right;
background: #ff0000;
color: white;
font-size: 16px;
border: 1px solid #666666;
border-right-width: 1px;
border-radius: 0.4em 0.4em 0.4em 0.4em;
margin-right: 8px;
}

#queue ul.mlistview > li.gap-after {
margin-bottom: 20px;
}

div.queued-round > p {
  margin-left: 10px;
  font-size: 18px;
  font-weight: bold;
}

div.queued-round > p > img.triangle {
  padding-right: 5px;
}

#queue ul > li > p {
margin: 0px;
width: 100%;
text-align: center;
}
#queue ul > li > p span.after-action {
  font-size: 16px;
  font-weight: normal;
}

p.squeeze {
display: none;
height: 0px;
}

#rounds-div {
  position: absolute;
  top: 150px;
  right: 0;
  width: 300px;
}

#rounds-ul.mlistview > li {
border: 3px blue dotted;
margin: 3px;
}

#rounds-ul.mlistview > li[data-roundid] {
border: 3px blue solid;
}

#rounds-ul.mlistview > li > input[type=button] {
  width: 50px;
  font-size: large;
  position: absolute;
  right: 10px;
  top: 5px;
  
  color:#ffffff;
  background: #2d2d2d; /* Old browsers */
  background: -moz-linear-gradient(top,  #424242 0%, #2d2d2d 100%); /* FF3.6+ */
  background: -webkit-gradient(linear, left top, left bottom, color-stop(0%,#424242), color-stop(100%,#2d2d2d)); /* Chrome,Safari4+ */
  background: -webkit-linear-gradient(top,  #424242 0%,#2d2d2d 100%); /* Chrome10+,Safari5.1+ */
  background: -o-linear-gradient(top,  #424242 0%,#2d2d2d 100%); /* Opera 11.10+ */
  background: -ms-linear-gradient(top,  #424242 0%,#2d2d2d 100%); /* IE10+ */
  background: linear-gradient(top,  #424242 0%,#2d2d2d 100%); /* W3C */

  border: 1px solid #666666;
  border-right-width: 1px;
  border-radius: 0.4em 0.4em 0.4em 0.4em;
}

li.width200.width200.width200 {
width: 256px;
}

#rounds-ul.mlistview > div.spacer {
height: 10px;
}

.hover {
  background-color: #ffffb4;
 }

select {
  font-size: 16px;
}

div.flipswitch.bucketed-multi {
  font-size: 24px;
  width: 10em;
  height: 2em;
}

div.flipswitch.bucketed-multi .on {
  text-indent: -6em;
}

div.flipswitch.bucketed-multi.checked {
  padding-left: 7em;
  width: 1.8125em;
}

/*
    - Height isn't right -- font size for ".block_buttons span" gets messed with
 */

</style>
<script type='text/javascript'>

  var g_all_scenes = <?php echo json_encode(all_scenes(),
                                            JSON_HEX_TAG | JSON_HEX_AMP | JSON_PRETTY_PRINT); ?>;
  var g_current_racing_scene = "<?php echo read_raceinfo('racing_scene', ''); ?>";

  var g_all_rounds = <?php echo json_encode(all_rounds_with_counts(/* by class: */false),
                                            JSON_HEX_TAG | JSON_HEX_AMP | JSON_PRETTY_PRINT); ?>;
  var g_queue = <?php
    $stmt = $db->query('SELECT queueid, seq, RacingQueue.classid, RacingQueue.round,'
                       .' n_times_per_lane, sceneid_at_finish, continue_racing,'
                       .' Classes.classid, class, round,'
                       .($use_groups ? "class || ', ' || " : "")
                       .'\'Round \' || round AS roundname'
                       .' FROM '.inner_join('RacingQueue', 'Classes',
                                            'RacingQueue.classid = Classes.classid')
                       .' ORDER BY seq');
    echo json_encode($stmt->fetchAll(PDO::FETCH_ASSOC),
                 JSON_HEX_TAG | JSON_HEX_AMP | JSON_PRETTY_PRINT);
  ?>;

  var g_classes = <?php echo json_encode($classes, 
                                         JSON_HEX_TAG | JSON_HEX_AMP | JSON_PRETTY_PRINT); ?>;

$(function() {
    $.each(g_queue,
           function(i, entry) {
             $('#queue-ul').append(make_queue_entry_li(entry, g_all_scenes));
           });

    build_rounds(g_queue, g_classes, g_all_rounds);
  });
</script>
<script type="text/javascript" src="js/racing-queue.js"></script>
</head>
<body>
<?php make_banner('Racing Queue', isset($_GET['back']) ? $_GET['back'] : 'coordinator.php'); ?>

<div id="main">

  <div id="queue">

    <div id="racing-scene-div">
      <label for="racing-scene">Racing scene:</label>
      <select id="racing-scene">
      </select>
    </div>

    <ul class="mlistview" id="queue-ul">
    </ul>
  </div>

  <div id="rounds-div">
    <p>Available rounds:</p>
    <ul class="mlistview" id="rounds-ul">
    </ul>
  </div>
</div>

<div id='new-roster-modal' class='modal_dialog block_buttons hidden'>
  <form>
    <p>Choose top</p>
    <input type="number" id="new_roster_top" value="3"/>

    <div class='no-buckets hidable'>
      <p>racers</p>
    </div>
    <div class='subgroup-buckets hidable'>
      <p>racers from</p>
      <div class="centered_flipswitch">
        <input type="checkbox" class="flipswitch bucketed-single"
                id="bucketed_subgroups"
                data-on-text="Each <?php echo subgroup_label(); ?>"
                data-off-text="Overall"/>
      </div>
    </div>
    <div class='group-buckets hidable'>
      <p>racers from</p>
      <div class="centered_flipswitch">
        <input type="checkbox" class="flipswitch bucketed-multi"
               id="bucketed_groups"
               data-on-text="Each <?php echo group_label(); ?>"
               data-off-text="Overall"/>
      </div>
    </div>

    <input type="submit" data-enhanced="true" value="Submit"/>
    <input type="button" data-enhanced="true" value="Cancel"
      onclick='close_modal("#new-roster-modal");'/>
  </form>
</div>

</body>
</html>
