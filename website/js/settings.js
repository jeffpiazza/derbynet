function on_form_submission() {
    $.ajax('action.php',
           {type: 'POST',
            data: $("#settings_form").serialize(),
            success: function(data) {
               var fail = data.documentElement.getElementsByTagName("failure");
               if (fail && fail.length > 0) {
                 console.log(data);
                 alert("Action failed: " + fail[0].textContent);
               } else {
                 g_form_modified = 0;
                 window.location.href = "setup.php";
               }
             },
             error: function(jqXHR, ajaxSettings, thrownError) {
               alert('Ajax error: ' + thrownError);
             }
           });
  return false;
};

function render_directory_status_icon(photo_dir_selector) {
    $.ajax('action.php',
           {type: 'GET',
            data: {query: 'file.stat',
                   path: $(photo_dir_selector).val()},
            success: function(data) {
                var icon_span = $(photo_dir_selector + '_icon');
                var msg_para = $(photo_dir_selector + '_message');
                var path = data.getElementsByTagName("path");
                if (path.length > 0) {
                    path = path[0];
                    if (path.getAttribute("directory") == "0" ||
                        path.getAttribute("readable") == "0") {
                        icon_span.html('<img src="img/status/trouble.png"/>');
                        msg_para.text('Directory does not exist or is not readable.');
                    } else if (path.getAttribute("writable") == "0") {
                        icon_span.html('<img src="img/status/readonly.png"/>');
                        msg_para.text('Directory is not writable.');
                    } else {
                        icon_span.html('<img src="img/status/ok.png"/>');
                        msg_para.text('');
                    }
                } else {
                    icon_span.html("");
                    msg_para.text('');
                }
            }
           });
}

function browse_for_photo_directory(photo_dir_selector) {
  var photo_dir = $(photo_dir_selector);
  var val = photo_dir.val();
  if (val == '') {
    val = photo_directory_base();  // Defined in settings.php
  }
  show_choose_directory_modal(val, function(path) {
    photo_dir.val(path);
    photo_dir.change();
  });
}

var g_form_modified = 0;

$(function() {

$("#settings_form").on("submit", on_form_submission);

$('#settings_form *').on("change", function() {
    g_form_modified = 1;
    $(this).closest(".settings_group").addClass("modified");
});

window.onbeforeunload = function() {
    if (g_form_modified == 1) {
        return "You have unsaved changes.";
    }
}

render_directory_status_icon("#photo-dir");
render_directory_status_icon("#car-photo-dir");

$("#photo-dir").on("change", function() { render_directory_status_icon("#photo-dir"); });
$("#car-photo-dir").on("change", function() { render_directory_status_icon("#car-photo-dir"); });

});
