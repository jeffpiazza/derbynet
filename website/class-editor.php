<?php @session_start();
require_once('inc/data.inc');
require_once('inc/banner.inc');
require_once('inc/schema_version.inc');
require_once('inc/authorize.inc');
require_once('inc/plural.inc');
require_permission(SET_UP_PERMISSION);
?><!DOCTYPE html>
<html>
<head>
<meta http-equiv="Content-Type" content="text/html; charset=UTF-8"/>
<title><?php echo group_label(); ?> Editor</title>
<link rel="stylesheet" type="text/css" href="css/jquery-ui.min.css"/>
<?php require('inc/stylesheet.inc'); ?>
<link rel="stylesheet" type="text/css" href="css/mobile.css"/>
<link rel="stylesheet" type="text/css" href="css/class-editor.css"/>
<script type="text/javascript" src="js/jquery.js"></script>
<script type="text/javascript" src="js/ajax-setup.js"></script>
<script type="text/javascript" src="js/jquery-ui.min.js"></script>
<script type="text/javascript" src="js/mobile.js"></script>
<script type="text/javascript" src="js/jquery.ui.touch-punch.min.js"></script>
<script type="text/javascript" src="js/modal.js"></script>
<script type="text/javascript" src="js/dashboard-ajax.js"></script>
<script type="text/javascript" src="js/class-editor.js"></script>
<script type="text/javascript">
function use_subgroups() { return <?php echo json_encode(use_subgroups()); ?>; }
function group_label() { return <?php echo json_encode(group_label()); ?>; }
function group_label_plural() { return <?php echo json_encode(plural(group_label())); ?>; }
function group_label_lc() { return <?php echo json_encode(group_label_lc()); ?>; }
function subgroup_label() { return <?php echo json_encode(subgroup_label()); ?>; }
function subgroup_label_plural() { return <?php echo json_encode(plural(subgroup_label())); ?>; }
function subgroup_label_lc() { return <?php echo json_encode(subgroup_label_lc()); ?>; }
$(function() { show_edit_all_classes_modal(); });
</script>
</head>
<body>
<?php
  // Since the back button is always obscured, the back button setting doesn't
  // really have much effect.
make_banner(group_label().' Editor', 'setup.php'); ?>

<div id="edit_all_classes_modal" class="modal_dialog hidden block_buttons">
  <form>
    <h3>Drag to Re-order <?php echo plural(group_label()); ?></h3>

    <div id="groups_container">
      <ul id="groups" class="mlistview has-alts">
      </ul>
    </div>

    <br/>

    <input type="button" value="Add <?php echo group_label(); ?>"
           onclick="show_add_class_modal();" />

    <input type="button" value="Add Aggregate"
           onclick="show_add_aggregate_modal();" />

    <br/>

    <input type="button" value="Close"
           onclick="close_edit_all_classes_modal();"/>
  </form>
</div>


<div id="add_class_modal" class="modal_dialog wide_modal hidden block_buttons">
  <form>
    <input type="hidden" name="action" value="json.class.add"/>

    <div id='aggregate-by-div' class="aggregate-only">
      <label for='aggregate-by-checkbox'>Aggregate by &nbsp;</label>
      <input id='aggregate-by-checkbox' type='checkbox' class='flipswitch'
            onchange='on_aggregate_by_change()'
            data-off-text="<?php echo group_label();?>"
            data-on-text="<?php echo subgroup_label();?>"/>
    </div>

    <h3>Add New <?php echo group_label(); ?></h3>
    <input id='add-class-name' name="name" type="text"/>

    <h3>Number of speed trophies:</h3>
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
    <h3>&nbsp;</h3>

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
    <h3><?php echo group_label(); ?> Name</h3>
    <input id="edit_class_name" name="name" type="text"/>

    <h3>Number of speed trophies:</h3>
    <select id="edit_class_ntrophies" name="ntrophies">
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
    <h3>&nbsp;</h3>

    <div id="completed_rounds_extension">
      <p><span id="completed_rounds_count"></span> completed round(s) exist for this class.</p>
    </div>

    <div id="constituent_extension">
      <p>Constituent of <span id="constituent_owner"></span>, possibly other aggregates.</p>
    </div>

    <div id="edit_ranks_extension" class="hidden">
      <h3>Drag to Re-order <?php echo plural(subgroup_label()); ?></h3>
      <div id="ranks_container">
      </div>
      <br/>
      <input type="button" value="Add <?php echo subgroup_label(); ?>"
             onclick="show_add_rank_modal();" />
      <br/>
    </div>

    <input type="submit"/>
    <input type="button" value="Cancel"
           onclick="close_edit_one_class_modal();"/>

    <div id="delete_class_extension">
    <input type="button" value="Delete <?php echo group_label(); ?>"
           class="delete_button"
           onclick="handle_delete_class();"/>
    </div>
  </form>
</div>

<div id="add_rank_modal" class="modal_dialog hidden block_buttons">
  <h3>Add New <?php echo subgroup_label(); ?></h3>
  <form>
    <input type="hidden" name="action" value="json.rank.add"/>
    <input type="hidden" name="classid"/>
    <input name="name" type="text"/>

    <input type="submit"/>
    <input type="button" value="Cancel"
           onclick="close_add_rank_modal();"/>
  </form>
</div>

<div id="edit_one_rank_modal" class="modal_dialog hidden block_buttons">
  <h3>New <?php echo subgroup_label(); ?> Name</h3>
  <form>
    <input id="edit_rank_name" name="name" type="text"/>

    <input type="submit"/>
    <input type="button" value="Cancel"
           onclick="close_edit_one_rank_modal();"/>

    <div id="delete_rank_extension">
    <input type="button" value="Delete <?php echo subgroup_label(); ?>"
           class="delete_button"
           onclick="handle_delete_rank();"/>
    </div>
  </form>
</div>

</body>
</html>
