// Requires dashboard-ajax.js

// Note that close_class_editor_modal returns to index.php upon dismissal of the
// modal dialog.

function show_class_editor_modal() {
    reload_class_list();
    show_modal("#class_editor_modal", function () { 
        return false;
    });
}

function close_class_editor_modal() {
    close_modal("#class_editor_modal");
    window.location='index.php';
}

function show_add_class_modal() {
    show_secondary_modal("#add_class_modal", function () {
        close_add_class_modal();
        $.ajax(g_action_url,
               {type: 'POST',
                data: $('#add_class_modal form').serialize(),
                success: function (data) {
                    reload_class_list();
                }});
        
        return false;
    });
}

function close_add_class_modal() {
    close_secondary_modal("#add_class_modal");
}

function edit_one_class(who) {
    $("#edit_class_name").val($(who).parent("li").text());
    show_edit_one_class_modal($(who).parent("li"));
}

function show_edit_one_class_modal(list_item) {
    show_secondary_modal("#edit_one_class_modal", function () {
        $.ajax(g_action_url,
               {type: 'POST',
                data: {action: 'class.edit',
                       classid: list_item.data('classid'),
                       name: $("#edit_class_name").val()},
                success: function () {
                    reload_class_list();
                }});

        close_edit_one_class_modal();
        return false;
    });
}

function close_edit_one_class_modal() {
    close_secondary_modal("#edit_one_class_modal");
}

function reload_class_list() {
    $.ajax(g_action_url,
           {type: 'GET',
            data: {query: 'class.list'},
            success: function(data) {
                repopulate_class_list(data);
            }});
}

function repopulate_class_list(data) {
    var classes = data.getElementsByTagName("class");
    if (classes.length > 0) {
        $("#groups").empty();
        for (var i = 0; i < classes.length; ++i) {
            var cl = classes[i];
            $("#groups").append(
                "<li data-classid='" + cl.getAttribute('classid') + "' class='ui-li-has-alt'>"
                    + "<p/>"
                    + "<a class='ui-btn ui-btn-icon-notext ui-icon-gear' onclick='edit_one_class(this);'></a>"
                    + "</li>\n");
            $("#groups li:last p").text(cl.getAttribute('name'));
        }
    }
}

$(function () {
    $("#groups").sortable({stop: function(event, ui) {
      var data = {action: 'class.order'};
      $("#class_editor_modal ul li").each(function(i) {
        data['classid_' + (i + 1)] = $(this).data('classid');
      });
      
      $.ajax(g_action_url,
             {type: 'POST',
              data: data});
    }});
});
