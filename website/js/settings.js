'use strict';

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
  PostSettingChange($("#unused-lane-mask"));
}

function on_max_runs_change() {
  $("#max-runs-per-car").val(document.getElementById('max-runs').checked ? 1 : 0);
  PostSettingChange($("#max-runs-per-car"));
}

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

// Respond to changes in group, supergroup, or subgroup label
function on_label_change() {
  $("span.supergroup-label").text($("#supergroup-label").val().toLowerCase());
  $("span.group-label").text($("#group-label").val().toLowerCase());
  $("span.subgroup-label").text($("#subgroup-label").val().toLowerCase());
}

// PostSettingChange(input) responds to a change in an <input> element by
// sending an ajax POST request with the input element's current value.  Handles
// checkboxes, too.

var PostSettingChange;

(function() {
  let next_train = 0;
  let values = {action: 'settings.write'};

  function maybe_post() {
    if (next_train == 0) {
      next_train = setTimeout(function() {
        next_train = 0;
        let d = values;
        values = {action: 'settings.write'};

        console.log('POSTing ' + JSON.stringify(d));

        $.ajax('action.php',
               {type: 'POST',
                data: d,
                success: function(data) {
                  var fail = data.documentElement.getElementsByTagName("failure");
                  if (fail && fail.length > 0) {
                    console.log(data);
                    alert("Action failed: " + fail[0].textContent);
                  }
                },
                error: function(jqXHR, ajaxSettings, thrownError) {
                  alert('Ajax error: ' + thrownError);
                }
               });
      }, 200);
    }
  }

  PostSettingChange = function(input) {
    let name = input.attr('name');
    if (typeof name == 'undefined' || name === false) {
      return;
    }

    if (input.attr('type') == 'checkbox') {
      values[name + '-checkbox'] = 'yes';
      if (input.is(':checked')) {
        values[name] = 1;
      } else {
        delete values[name];
      }
    } else {
      values[name] = input.val();
    }

    maybe_post();
  };

})();

var g_form_modified = 0;

$(function() {

  $("#n-lanes").on("keyup mouseup", on_lane_count_change);
  on_lane_count_change();

  $("#supergroup-label").on("keyup mouseup", on_label_change);
  $("#group-label").on("keyup mouseup", on_label_change);
  $("#subgroup-label").on("keyup mouseup", on_label_change);

  $('#settings_form input').on('change', function(e) {
    PostSettingChange($(this));
  });
  $('#settings_form input[type!="checkbox"]').on('input', function(e) {
    PostSettingChange($(this));
  });

  render_directory_status_icon("#photo-dir");
  render_directory_status_icon("#car-photo-dir");

  $("#photo-dir").on("change", function() { render_directory_status_icon("#photo-dir"); });
  $("#car-photo-dir").on("change", function() { render_directory_status_icon("#car-photo-dir"); });

});
