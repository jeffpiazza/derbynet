<?php @session_start();
require_once('inc/data.inc');
require_once('inc/authorize.inc');
session_write_close();
require_once('inc/classes.inc');
require_once('inc/banner.inc');
require_once('inc/partitions.inc');
require_once('inc/schema_version.inc');
require_permission(SET_UP_PERMISSION);

if (schema_version() < PARTITION_SCHEMA) {
  header('Location: setup.php');
  exit(0);
}


?><!DOCTYPE html>
<html>
<head>
<meta http-equiv="Content-Type" content="text/html; charset=UTF-8"/>
<title>Racing Groups Editor</title>
<?php require('inc/stylesheet.inc'); ?>
<script type="text/javascript" src="js/jquery.js"></script>
<script type="text/javascript" src="js/jquery-ui.min.js"></script>
<script type="text/javascript" src="js/ajax-setup.js"></script>
<script type="text/javascript" src="js/jquery.ui.touch-punch.min.js"></script>
<script type="text/javascript" src="js/dashboard-ajax.js"></script>
<script type="text/javascript" src="js/mobile.js"></script>
<script type="text/javascript" src="js/modal.js"></script>
<script type="text/javascript" src="js/racing-groups.js"></script>
<script type="text/javascript" src="js/racing-groups-add.js"></script>
<script type="text/javascript" src="js/racing-groups-edit.js"></script>
<script type="text/javascript">
$(function() {
    var rule = <?php echo json_encode(group_formation_rule()); ?>;
    $("input[name='form-groups-by'][value='" + rule + "']").prop('checked', true);
    mobile_radio_refresh($("input[name='form-groups-by']"));
  });
</script>
<link rel="stylesheet" type="text/css" href="css/mobile.css"/>
<link rel="stylesheet" type="text/css" href="css/racing-groups.css"/>
</head>
<body>
<?php make_banner('Racing Groups', 'setup.php'); ?>

<div id="below-banner">

<div id="race-rules">


<input id="by-partition-radio" type="radio" name="form-groups-by" value="by-partition"/>
<label for="by-partition-radio">Race each
       <span class="partition-label-lc"><?php echo partition_label_lc(); ?></span>
       as a group</label>
<input id="one-group-radio" type="radio" name="form-groups-by" value="one-group"/>
<label for="one-group-radio">Race as one big group</label>
<input id="custom-group-radio" type="radio" name="form-groups-by" value="custom"/>
<label for="custom-group-radio">Custom racing groups</label>


<div class="switch">
  <label for="use-subgroups">Use Subgroups?</label>
  <input id="use-subgroups" type="checkbox" class="flipswitch"
       data-on-text="Yes" data-off-text="No"
         <?php if (use_subgroups()) echo "checked=\"checked\""; ?>/>
</div>

<div class="labels">
  <label for="supergroup-label">The full roster is a (or the)</label>
  <input id="supergroup-label" name="supergroup-label" type="text" class="not-mobile"
               value="<?php echo supergroup_label(); ?>"/>,
  <label for="partition-label">and a sub-division is a(n)</label>
  <input id="partition-label" name="partition-label" type="text" class="not-mobile"
               value="<?php echo partition_label(); ?>"/>.
</div>

<h3>Awards</h3>

<div class="n_default_trophies">
   <input id="n-pack" name="n-pack-trophies" type="number" min="0" max="20" class="not-mobile"
               value="<?php echo read_raceinfo('n-pack-trophies', 3); ?>"/>
   <label for="n-pack">speed trophies at the
               <span class="supergroup-label"><?php echo supergroup_label_lc(); ?></span> level</label>
</div>

<div id="pack_agg_div">
  <span class="supergroup-label"><?php echo supergroup_label(); ?></span> standings:
  <br/>
  <input id="pack-ok" type="radio" name="pack-agg" class="not-mobile" value="0"/>
  <label for="pack-ok">Calculate normally</label>
  <br/>
  <input id="pack-no" type="radio" name="pack-agg" class="not-mobile" value="-1"/>
  <label for="pack-no"><span id="dimmable-for-pack-no"><?php echo "Don't calculate"; ?></span>
         <span id="why-not-pack-no">(needed for
             <span class="supergroup-label"><?php echo supergroup_label(); ?></span>
             trophies)</span></label>
  <!-- div pack-agg-option -->
</div>

<div class="n_default_trophies">
  <input id="n-den" name="n-den-trophies" type="number" min="0" max="20" class="not-mobile"
            value="<?php echo read_raceinfo('n-den-trophies', 3); ?>"/>
  <label for="n-den">speed trophies per group</label>
</div>

<div class="n_default_trophies">
        <input id="n-rank" name="n-rank-trophies" type="number" min="0" max="20" class="not-mobile"
               value="<?php echo read_raceinfo('n-rank-trophies', 0); ?>"/>
        <label for="n-rank">speed trophies per subgroup</label>
</div>

</div><!-- race-rules -->

<div id="race-structure">

<p class="instructions" id="drag-groups">Drag <span class="group-color">&nbsp;</span> groups to reorder.</p>
<p class="instructions" id="drag-subgroups">Drag <span class="subgroup-color">&nbsp;</span> subgroups
        to reorder<span id="or-to-move"> or to move to another group</span>.</p>

<ul id="all-groups">

  <li id='new-group' class='group' data-classid="-1">
    <p class='class-name'>New Group</p>
    <ul class='subgroups'></ul>
  </li>
</ul>

<div class="block_buttons add_button">
  <input id="add-partition-button" class="modest-button" type="button"
         value="Add <?php echo partition_label(); ?>"/>
</div>

<ul id="aggregate-groups" class="mlistview">
</ul>

<div class="block_buttons add_button">
  <input id="add-aggregate-button" class="modest-button" type="button"
         value="Add Aggregate"/>
</div>

</div><!-- race-structure -->

</div><!-- below-banner -->



<div id="add_class_modal" class="modal_dialog wide_modal hidden block_buttons">
  <form>
    <input type="hidden" name="action" value="class.add"/>

    <div id='aggregate-by-div' class="aggregate-only">
      <label for='aggregate-by-checkbox'>Aggregate by &nbsp;</label>
      <input id='aggregate-by-checkbox' type='checkbox' class='flipswitch'
            onchange='on_aggregate_by_change()'
            data-off-text="<?php echo group_label();?>"
            data-on-text="<?php echo subgroup_label();?>"/>
    </div>

    <h3>Add New <span class="group-label"><?php echo group_label(); ?></span></h3>
    <input id='add-class-name' name="name" type="text"/>

   <div class="ntrophies">
    <label for='add-class-ntrophies'>Number of speed trophies:</label>
    <select id='add-class-ntrophies' name="ntrophies">
      <option value="-1" selected="selected">Default</option>
      <option>0</option>
      <option>1</option>
      <option>2</option>
      <option>3</option>
      <option>4</option>
      <option>5</option>
      <option>6</option>
      <option>7</option>
      <option>8</option>
      <option>9</option>
      <option>10</option>
    </select>
   </div>

    <div id='constituent-clip' class='aggregate-only'>
      <div id='constituent-div'>
        <div id='constituent-classes'></div>
        <div id='constituent-subgroups'></div>
      </div>
    </div>

    <input type="submit"/>
    <input type="button" value="Cancel"
           onclick="close_add_class_modal();"/>
  </form>
</div>

<div id="edit_one_class_modal" class="modal_dialog hidden block_buttons">
  <form>
    <h3><span class="group-label"><?php echo group_label(); ?></span> Name</h3>
    <input id="edit_class_name" name="name" type="text"/>

   <div class="ntrophies">
    <label for='edit_class_ntrophies'>Number of speed&nbsp;trophies:</label>
    <select id='edit_class_ntrophies' name='ntrophies'>
      <option value="-1">Default</option>
      <option>0</option>
      <option>1</option>
      <option>2</option>
      <option>3</option>
      <option>4</option>
      <option>5</option>
      <option>6</option>
      <option>7</option>
      <option>8</option>
      <option>9</option>
      <option>10</option>
    </select>
</div>

    <div id="completed_rounds_extension">
      <p><span id="completed_rounds_count"></span> completed round(s) exist for this class.</p>
    </div>

    <div id="constituent_extension">
      <p>Constituent of <span id="constituent_owner"></span>, possibly other aggregates.</p>
    </div>

    <div id="edit_ranks_extension" class="hidden">
      <input id="add_rank_button" type="button"
             value="Add <?php echo subgroup_label(); ?>"
             onclick="show_add_rank_modal();" />
      <br/>
    </div>

    <input type="submit"/>
    <input type="button" value="Cancel"
           onclick="close_edit_one_class_modal();"/>

    <div id="delete_class_extension">
    <input id="delete_class_button" type="button"
           value="Delete <?php echo group_label(); ?>"
           data-label="<?php echo group_label(); ?>"
           class="delete_button"
           onclick="handle_delete_class(this);"/>
    </div>
  </form>
</div>

<div id="edit_one_rank_modal" class="modal_dialog hidden block_buttons">
  <h3>New <span class="subgroup-label"><?php echo subgroup_label(); ?></span> Name</h3>
  <form>
    <input id="edit_rank_name" name="name" type="text"/>

    <input type="submit"/>
    <input type="button" value="Cancel"
           onclick="close_edit_one_rank_modal();"/>

    <div id="delete_rank_extension">
    <input id="delete_rank_button" type="button"
           value="Delete <?php echo subgroup_label(); ?>"
           data-label="<?php echo subgroup_label(); ?>"
           class="delete_button"
           onclick="handle_delete_rank(this);"/>
    </div>
  </form>
</div>


<div id="add_partition_modal" class="modal_dialog hidden block_buttons">
  <h3>Add New <span class="partition-label"><?php echo partition_label(); ?></span></h3>
  <form>
    <input type="hidden" name="action" value="partition.add"/>
    <input name="name" type="text"/>

    <input type="submit"/>
    <input type="button" value="Cancel"
           onclick="close_add_partition_modal();"/>
  </form>
</div>

<div id="edit_one_partition_modal" class="modal_dialog hidden block_buttons">
  <h3>New <span class="partition-label"><?php echo partition_label(); ?></span> Name</h3>
  <form>
    <input id="edit_partition_name" name="name" type="text"/>

    <input type="submit"/>
    <input type="button" value="Cancel"
           onclick="close_edit_one_partition_modal();"/>

    <div id="delete_partition_extension">
    <input id="delete_partition_button" type="button"
           value="Delete <?php echo partition_label(); ?>"
           data-label="<?php echo partition_label(); ?>"
           class="delete_button"
           onclick="handle_delete_partition(this);"/>
    </div>
  </form>
</div>

</body>
</html>
