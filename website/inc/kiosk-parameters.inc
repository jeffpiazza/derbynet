<?php require_once('inc/path-info.inc'); ?>

<!-- Used for configuring both slideshow and please-check-in -->
<div id='config_classes_modal' class="modal_dialog hidden block_buttons">
  <form>
    <div id="slideshow_div">
      <div>
        <label for="title_text">Title text:</label>
        <input type="text" id="title_text"/>
      </div>
      <div id="slideshow_subdir_div">
         <?php
             $subdirs = find_alternate_slides_directories();
             if (count($subdirs) > 0) {
               echo "<select id='slideshow_subdir'>\n";
               echo "<option value=''>(Default slideshow)</option>\n";
               foreach ($subdirs as $sub) {
                 echo "<option>".htmlspecialchars($sub, ENT_QUOTES, 'UTF-8')."</option>\n";
               }
               echo "</select>\n";
             }
         ?>
      </div>
    </div>
    <div id="classids_div">
    <?php
        $stmt = $db->prepare('SELECT classid, class'
                             .' FROM Classes'
                             // .' WHERE EXISTS(SELECT 1 FROM RegistrationInfo'
                             // .'              WHERE RegistrationInfo.classid = Classes.classid)'
                             .' ORDER BY '.(schema_version() >= 2
                                            ? 'sortorder, ' : '')
                             .'class');
        $stmt->execute(array());

        foreach ($stmt as $row) {
          echo '<input type="checkbox" name="class-'.$row['classid'].'"'
               .' class="flipswitch"'
               .' id="config-class-'.$row['classid'].'"'
               .' data-classid="'.$row['classid'].'"'
               .'/>'."\n";
          echo '<label for="config-class-'.$row['classid'].'">'
              .htmlspecialchars($row['class'], ENT_QUOTES, 'UTF-8')
              .'</label>'."\n";
          echo "<br/>\n";
        }
    ?>
    </div>
    <input type="submit" value="Configure Kiosk"/>
    <input type="button" value="Cancel"
      onclick='close_modal("#config_classes_modal");'/>
  </form>
</div>

<div id='config_qrcode_modal' class="modal_dialog wide_modal hidden block_buttons">
  <form>
    <div>
      <label for="qrcode-title">Page Title</label>
      <input id="qrcode-title" type="text"/>
    </div>
    <div>
      <label for="qrcode-content">QR Code</label>
      <input id="qrcode-content" type="text"/>
    </div>
    <input type="submit" value="Configure Kiosk"/>
    <input type="button" value="Cancel"
      onclick='close_modal("#config_qrcode_modal");'/>
  </form>
</div>

