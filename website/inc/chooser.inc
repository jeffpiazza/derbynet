<div id="chooser_modal" class="modal_dialog hidden block_buttons">
  <form>
    <input id="chooser_directory_path" type="hidden"/>
    <input id="chooser_file_name" type="text" data-wrapper-class="chooser_top_text_input"/>
    <select id="chooser_directories_above"></select>
    <div id="chooser_directory_content_container">
      <ul id="chooser_directory_content" data-role="listview" class="ui-listview"/>
    </div>
    <input id="chooser_add_directory" type="button" data-enhanced="true"
           value="New Directory"/>
    <input type="submit" value="Choose" data-enhanced="true"/>
    <input type="button" value="Cancel" data-enhanced="true"
           onclick="close_chooser_modal();"/>
  </form>
</div>

<div id="chooser_add_directory_modal" class="modal_dialog hidden block_buttons">
  <form>
    <h3>Add New Directory:</h3>
    <input id="chooser_new_directory_name" type="text"/>
    <input type="submit" value="Choose" data-enhanced="true"/>
    <input type="button" value="Cancel" data-enhanced="true"
           onclick="chooser_close_add_directory_modal();"/>
  </form>
</div>
