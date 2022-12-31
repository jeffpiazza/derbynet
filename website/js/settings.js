'use strict';

function on_lane_count_change() {
  $("#lanes-in-use").empty();
  var nlanes = $("#n-lanes").val();
  var mask = $("#unused-lane-mask").val();
  for (var i = 0; i < nlanes; ++i) {
    var bit = 1 << i;
    $("#lanes-in-use").append(" " + (i + 1) + ":");
    var img_src = (mask & bit) ? 'img/lane_closed.png' : 'img/lane_open.png';
    $("#lanes-in-use").append("<img data-bit='" + bit + "' src='" + img_src + "'/>");
  }

  // In case the lane count decreased, clear any higher-order bits as they're no
  // longer meaningful
  $("#unused-lane-mask").val(mask & ~(-1 << nlanes));
  
  $("#lanes-in-use img").on('click', on_lane_click);
}

function on_lane_click(event) {
  if ($("#unused-lane-mask").prop('disabled')) {
    return;
  }
  var mask = $("#unused-lane-mask").val();
  var target = $(event.currentTarget);
  var bit = target.attr('data-bit');
  mask ^= bit;
  target.attr('src', (mask & bit) ? 'img/lane_closed.png' : 'img/lane_open.png');

  $("#unused-lane-mask").val(mask);
  PostSettingChange($("#unused-lane-mask"));
}

function on_linger_time_change() {
  $("#now-racing-linger-ms").val($("#now-racing-linger-sec").val() * 1000);
  PostSettingChange($("#now-racing-linger-ms"));
  return false;
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
            console.log(data);
            var icon_span = $(photo_dir_selector + '_icon');
            var msg_para = $(photo_dir_selector + '_message');
            if (data.hasOwnProperty('stat')) {
              var stat = data.stat;
              if (!stat.isdir || !stat.readable) {
                icon_span.html('<img src="img/status/trouble.png"/>');
                msg_para.text('Directory does not exist or is not readable.');
              } else if (!stat.writable) {
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

function on_supergroup_label_change() {
  $("span.supergroup-label").text($("#supergroup-label").val().toLowerCase());
}

// PostSettingChange(input) responds to a change in an <input> element by
// sending an ajax POST request with the input element's current value.  Handles
// checkboxes, too.

var PostSettingChange;

(function() {
  var next_train = 0;
  var values = {action: 'settings.write'};

  function maybe_post() {
    if (next_train == 0) {
      next_train = setTimeout(function() {
        next_train = 0;
        var d = values;
        values = {action: 'settings.write'};

        console.log('POSTing ' + JSON.stringify(d));

        $.ajax('action.php',
               {type: 'POST',
                data: d,
                success: function(data) {
                  if (data.outcome.summary == 'failure') {
                    console.log(data);
                    alert("Action failed: " + data.outcome.description);
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
    if ($(input).hasClass('do-not-post')) {
      return;
    }
    var name = input.attr('name');
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

$(function() {

  $("#n-lanes").on("keyup mouseup", on_lane_count_change);
  on_lane_count_change();

  $("#now-racing-linger-sec").on("keyup mouseup", on_linger_time_change);

  $("#supergroup-label").on("keyup mouseup", on_supergroup_label_change);

  $('#settings_form input, #settings_form select').on('change', function(e) {
    PostSettingChange($(this));
  });
  $('#settings_form input[type!="checkbox"]').on('input', function(e) {
    PostSettingChange($(this));
  });

  if ($("#photo-dir").length > 0) {
    render_directory_status_icon("#photo-dir");
    render_directory_status_icon("#car-photo-dir");
    render_directory_status_icon("#video-dir");
  }

  $("#photo-dir").on("change", function() { render_directory_status_icon("#photo-dir"); });
  $("#car-photo-dir").on("change", function() { render_directory_status_icon("#car-photo-dir"); });
  $("#video-dir").on("change", function() { render_directory_status_icon("#video-dir"); });

});
