
function on_lane_count_change() {
  $("#lanes-in-use").empty();
  var nlanes = $("#n-lanes").val();
  var mask = $("#unused-lane-mask").val();
  for (var i = 0; i < nlanes; ++i) {
    var bit = 1 << i;
    $("#lanes-in-use").append(" " + (i + 1) + ":");
    if (mask & bit) {
      $("#lanes-in-use").append("<img data-bit='" + bit + "' src='img/lane_closed.png'/>");
    } else {
      $("#lanes-in-use").append("<img data-bit='" + bit + "' src='img/lane_open.png'/>");
    }
  }

  // In case the lane count decreased, clear any higher-order bits as they're no
  // longer meaningful
  $("#unused-lane-mask").val(mask & ~(-1 << nlanes));
  
  $("#lanes-in-use img").on('click', on_lane_click);
}

function on_lane_click(event) {
  var mask = $("#unused-lane-mask").val();
  var target = $(event.currentTarget);
  var bit = target.attr('data-bit');
  if ((mask & bit) == 0) {  // open -> closed
    target.attr('src', 'img/lane_closed.png');
    mask |= bit;
  } else {
    target.attr('src', 'img/lane_open.png');
    mask &= ~bit;
  }

  $("#unused-lane-mask").val(mask);
  g_form_modified = 1;
  target.closest(".settings_group").addClass("modified");
}

function on_max_runs_change() {
  $("#max-runs-per-car").val(document.getElementById('max-runs').checked ? 1 : 0);
  return false;
}

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

$("#n-lanes").on("keyup mouseup", on_lane_count_change);
on_lane_count_change();  
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
