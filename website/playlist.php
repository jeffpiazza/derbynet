<?php @session_start();

// $_GET['back'] if "Back" button should go to another page, otherwise coordinator.php.

require_once('inc/data.inc');
require_once('inc/authorize.inc');
session_write_close();
require_once('inc/partitions.inc');
require_once('inc/banner.inc');
require_once('inc/schema_version.inc');
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

$all_rounds_by_class = array();
foreach (all_rounds_with_counts() as $round) {
  if (!isset($all_rounds_by_class[$round['classid']])) {
    $all_rounds_by_class[$round['classid']] = array();
  }
  $all_rounds_by_class[$round['classid']][] = $round;
}
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
  var g_group_label = <?php
      echo json_encode(group_label(), JSON_HEX_TAG | JSON_HEX_AMP); ?>;
  var g_subgroup_label = <?php
      echo json_encode(subgroup_label(), JSON_HEX_TAG | JSON_HEX_AMP); ?>;

  var g_all_scenes = <?php
      echo json_encode(all_scenes(),
                       JSON_HEX_TAG | JSON_HEX_AMP | JSON_PRETTY_PRINT | JSON_NUMERIC_CHECK); ?>;
  var g_current_racing_scene = <?php
      echo json_encode(read_raceinfo('racing_scene', ''),
                       JSON_HEX_TAG | JSON_HEX_AMP  | JSON_NUMERIC_CHECK); ?>;
  var g_current_round = <?php
      echo json_encode(get_running_round(),
                       JSON_HEX_TAG | JSON_HEX_AMP  | JSON_PRETTY_PRINT | JSON_NUMERIC_CHECK); ?>;

  var g_all_rounds = <?php
      echo json_encode($all_rounds_by_class,
                       JSON_HEX_TAG | JSON_HEX_AMP  | JSON_PRETTY_PRINT | JSON_NUMERIC_CHECK); ?>;
  var g_queue = <?php
    $stmt = $db->query('SELECT queueid, seq, Playlist.classid, Playlist.round,'
                       .' bucket_limit, bucketed, n_times_per_lane,'
                       .' sceneid_at_finish, continue_racing,'
                       .' Classes.classid, class, round,'
                       .($use_groups ? "class || ', ' || " : "")
                       .'\'Round \' || round AS roundname'
                       .' FROM '.inner_join('Playlist', 'Classes',
                                            'Playlist.classid = Classes.classid')
                       .' ORDER BY seq');
    echo json_encode($stmt->fetchAll(PDO::FETCH_ASSOC),
                 JSON_HEX_TAG | JSON_HEX_AMP  | JSON_PRETTY_PRINT | JSON_NUMERIC_CHECK);
  ?>;

  var g_classes = <?php
      echo json_encode($classes, 
                       JSON_HEX_TAG | JSON_HEX_AMP  | JSON_PRETTY_PRINT | JSON_NUMERIC_CHECK); ?>;

$(function() {
    $.each(g_queue,
           function(i, entry) { append_playlist_entry_li(entry, g_all_scenes); });
    maybe_change_playlist_message();
    build_rounds(g_queue, g_classes, g_all_rounds);
    build_after_action_select("#after-action-sel", g_all_scenes);
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
      <p id='racing-scene-psa' class='hidden'>
        It's usually a good idea to set a racing scene if you're using playlists.
      </p>
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

<div id='add-to-queue-modal' class='modal_dialog block_buttons hidden'>
  <form>
    <div id='new-roster-div'>
      <p>Choose top</p>
      <input type="number" id="new-roster-top" value="3"/>

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
    </div>

    <label for='schedule-reps'>Runs per lane:</label>
    <select id='schedule-reps'>
      <option>1</option>
      <option>2</option>
      <option>3</option>
      <option>4</option>
      <option>5</option>
      <option>6</option>
    </select>

    <label for='after-action-sel'>And then:</label>
    <select id='after-action-sel'>
      <!-- Options populated by javascript at the top of this file. -->
    </select>

    <input type="submit" value="Submit"/>
    <input type="button" value="Cancel"
      onclick='close_modal("#add-to-queue-modal");'/>
  </form>
</div>

</body>
</html>
