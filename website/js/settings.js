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
                 window.location.href = "index.php";
               }
             },
             error: function(jqXHR, ajaxSettings, thrownError) {
               alert('Ajax error: ' + thrownError);
             }
           });
  return false;
};

function render_directory_status_icon(photo_dir_selector) {
    console.log("Updating status icon for " + photo_dir_selector);
    $.ajax('action.php',
           {type: 'GET',
            data: {query: 'file-path',
                   path: $(photo_dir_selector).val()},
            success: function(data) {
                var path = data.getElementsByTagName("path");
                if (path.length > 0) {
                    path = path[0];
                    var icon_span = $(photo_dir_selector + '_icon');
                    var msg_para = $(photo_dir_selector + '_message');
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
