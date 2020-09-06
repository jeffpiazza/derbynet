<?php @session_start();

// $_GET['back'] if "Back" button should go to another page, otherwise coordinator.php.

require_once('inc/data.inc');
require_once('inc/banner.inc');
require_once('inc/schema_version.inc');
require_once('inc/authorize.inc');
require_once('inc/kiosks.inc');
require_once('inc/scenes.inc');
require_once('inc/classes.inc');
require_once('inc/playlist.inc');
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
<title>Rounds Playlist</title>
<?php require('inc/stylesheet.inc'); ?>
<link rel="stylesheet" type="text/css" href="css/jquery-ui.min.css"/>
<link rel="stylesheet" type="text/css" href="css/mobile.css"/>
<link rel="stylesheet" type="text/css" href="css/playlist.css"/>
<script type="text/javascript" src="js/jquery.js"></script>
<script type="text/javascript" src="js/jquery-ui.min.js"></script>
<script type="text/javascript" src="js/jquery.ui.touch-punch.min.js"></script>
<script type="text/javascript" src="js/mobile.js"></script>
<script type='text/javascript' src="js/modal.js"></script>
<script type='text/javascript'>

  var g_use_subgroups = <?php echo use_subgroups() ? "true" : "false"; ?>;

  var g_all_scenes = <?php echo json_encode(all_scenes(),
                                            JSON_HEX_TAG | JSON_HEX_AMP | JSON_PRETTY_PRINT); ?>;
  var g_current_racing_scene = "<?php echo read_raceinfo('racing_scene', ''); ?>";

  var g_all_rounds = <?php echo json_encode(all_rounds_with_counts(/* by class: */false),
                                            JSON_HEX_TAG | JSON_HEX_AMP | JSON_PRETTY_PRINT); ?>;
  var g_queue = <?php
    $stmt = $db->query('SELECT queueid, seq, Playlist.classid, Playlist.round,'
                       .' n_times_per_lane, sceneid_at_finish, continue_racing,'
                       .' Classes.classid, class, round,'
                       .($use_groups ? "class || ', ' || " : "")
                       .'\'Round \' || round AS roundname'
                       .' FROM '.inner_join('Playlist', 'Classes',
                                            'Playlist.classid = Classes.classid')
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
    maybe_change_queue_message();
    build_rounds(g_queue, g_classes, g_all_rounds);
  });
</script>
<script type="text/javascript" src="js/playlist.js"></script>
</head>
<body>
<?php make_banner('Rounds Playlist', isset($_GET['back']) ? $_GET['back'] : 'coordinator.php'); ?>

<div id="main">

  <div id="queue">

    <div id="racing-scene-div">
      <label for="racing-scene">Racing scene:</label>
      <select id="racing-scene">
      </select>
    </div>

    <p id="top-of-queue"></p>
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
