// Requires dashboard-ajax.js
// Requires modal.js

// var g_order specified in checkin.php

// For the photo_modal dialog, this boolean controls whether the racer gets
// checked in as a side-effect of uploading a photo.
var g_check_in;

// Default values for autocrop, depending on repo.  If user changes the setting,
// that setting endures for the repo for as long as the page is loaded.
var g_autocrop_head = true;
var g_autocrop_car = false;

var g_cameras = new Array();
var g_cameraIndex = 0;
var g_width = 640;
var g_height = 480;

function set_autocrop_state(repo) {
  $("#autocrop").prop('checked', repo == 'head' ? g_autocrop_head : g_autocrop_car);
  // TODO $("#autocrop").flipswitch("refresh");
}

function preserve_autocrop_state() {
  var repo = $("#photo_modal_repo").val();
  if (repo == 'head') {
    g_autocrop_head = $("#autocrop").prop('checked');
  } else {
    g_autocrop_car = $("#autocrop").prop('checked');
  }
}

// This executes when a checkbox for "Passed" is clicked.
function handlechange_passed(cb, racer) {
    // cb is the checkbox element, with name "passed-" plus the racer id, e.g., passed-1234
    if (!cb.checked && !confirm("Are you sure you want to unregister " + racer + "?")) {
	    cb.checked = true;
	    return;
    }
    // 7 = length of "passed-" prefix
    var racer = cb.name.substring(7);
    var value = cb.checked ? 1 : 0;

    $.ajax(g_action_url,
           {type: 'POST',
            data: {action: 'racer.pass',
                   racer: racer,
                   value: value},
           });
}

// This executes when a checkbox for "Exclusively by Scout" is clicked.
function handlechange_xbs(cb) {
    // cb is the checkbox element, with name "xbs-" plus the racer id, e.g., xbs-1234
    // 4 = length of "xbs-" prefix
    var racer = cb.name.substring(4);
    var value = cb.checked ? 1 : 0;

    $.ajax(g_action_url,
           {type: 'POST',
            data: {action: 'json.award.xbs',
                   racer: racer,
                   value: value},
           });
}

function show_edit_racer_form(racerid) {
  var first_name = $('#firstname-' + racerid).text();
  var last_name = $('#lastname-' + racerid).text();
  var car_no = $('#car-number-' + racerid).text();
  var car_name = $('#car-name-' + racerid).text();

  var rankid = $('#class-' + racerid).attr('data-rankid');

  $("#edit_racer").val(racerid);

  $("#edit_firstname").val(first_name);
  $("#edit_lastname").val(last_name);

  $("#edit_carno").val(car_no);
  $("#edit_carname").val(car_name);

  var edit_rank = $("#edit_rank");
  edit_rank.val(rankid);
  // I think it's a bug in jquery-mobile that an explicit change
  // event is required; setting the val above should be sufficient
  // to cause an update.
  edit_rank.change();

  $("#eligible").prop("checked", $('#lastname-' + racerid).attr("data-exclude") == 0);
  $("#eligible").trigger("change", true);

  $("#delete_racer_extension").removeClass('hidden');

  show_modal("#edit_racer_modal", function(event) {
      handle_edit_racer();
      return false;
  });

  $("#edit_carno").focus();
}

function show_new_racer_form() {
  $("#edit_racer").val(-1);

  $("#edit_firstname").val("");
  $("#edit_firstname").focus();

  $("#edit_lastname").val("");

  $("#edit_carno").val(9999);
  $("#edit_carname").val("");

  $("#eligible").prop("checked", true);
  $("#eligible").trigger("change", true);

  $("#delete_racer_extension").addClass('hidden');
  
  show_modal("#edit_racer_modal", function(event) {
      handle_edit_racer();
      return false;
  });
}

function handle_edit_racer() {
  close_modal("#edit_racer_modal");

  var racerid = $("#edit_racer").val();

  var new_firstname = $("#edit_firstname").val().trim();
  var new_lastname = $("#edit_lastname").val().trim();
  var new_carno = $("#edit_carno").val().trim();
  var new_carname = $("#edit_carname").val().trim();

  var rank_picker = $("#edit_rank");
  var new_rankid = rank_picker.val();

  var rank_option = $('[value="' + new_rankid + '"]', rank_picker);
  var new_classname = rank_option.attr('data-class');
  var new_rankname = rank_option.attr('data-rank');

  var exclude = $("#eligible").is(':checked') ? 0 : 1;

  $.ajax(g_action_url,
         {type: 'POST',
          data: {action: racerid >= 0 ? 'racer.edit' : 'racer.new',
                 racer: racerid,
                 firstname: new_firstname,
                 lastname: new_lastname,
                 carno: new_carno,
                 carname: new_carname,
                 rankid: new_rankid,
                 exclude: exclude},
            success: function(data) {
                var warnings = data.getElementsByTagName('warning');
                if (warnings && warnings.length > 0) {
                  window.alert("WARNING: " + warnings[0].childNodes[0].nodeValue);
                }
                var new_row_elements = data.getElementsByTagName('new-row');
                if (new_row_elements.length > 0) {
	                tb = $(".main_table tbody");
                    for (var j = 0; j < new_row_elements.length; ++j) {
                        var tr_elements = new_row_elements[j].getElementsByTagName('tr');
                        for (var jj = 0; jj < tr_elements.length; ++jj) {
                            var new_tr = document.createElement('tr');
	                        tb.get(0).appendChild(new_tr);
                            new_tr.outerHTML = (new XMLSerializer()).serializeToString(tr_elements[jj]);
                            tb.trigger('create');
                        }
                    }
                } else {
                    $("#firstname-" + racerid).text(new_firstname);
                    var ln = $("#lastname-" + racerid);
                    ln.text(new_lastname);
                    ln.attr("data-exclude", exclude);
                    ln.parents('tr').toggleClass('exclude-racer', exclude == 1);
                    $("#car-number-" + racerid).text(new_carno);
                    $("#car-name-" + racerid).text(new_carname);

                    $('#class-' + racerid).attr('data-rankid', new_rankid);
                    $('#class-' + racerid).text(new_classname);
                    $('#rank-' + racerid).text(new_rankname);
                }

                sort_checkin_table();
            },
           });
}

function handle_delete_racer() {
  close_modal("#edit_racer_modal");

  var racerid = $("#edit_racer").val();

  var first_name = $('#firstname-' + racerid).text();
  var last_name = $('#lastname-' + racerid).text();
  var car_no = $('#car-number-' + racerid).text();

  if (confirm("Really delete car #" + car_no + ": " + first_name + " " + last_name + "?")) {
    $.ajax(g_action_url,
           {type: 'POST',
            data: {action: 'racer.delete',
                   racer: racerid}
           });
  }

  return false;
}


function show_bulk_form() {
  show_modal("#bulk_modal", function(event) {
      return false;
  });
}

function bulk_check_in(value) {
  close_modal("#bulk_modal");
  $("#bulk_details_title").text(value ? "Bulk Check-In" : "Bulk Check-In Undo");
  $("#who_label").text(value ? "Check in racers in" : "Undo check-in of racers in");
  $("#bulk_details div.hidable").addClass("hidden");

  show_secondary_modal("#bulk_details_modal", function(event) {
    close_secondary_modal("#bulk_details_modal");
    $.ajax(g_action_url,
           {type: 'POST',
            data: {action: 'racer.bulk',
                   what: 'checkin',
                   who: $("#bulk_who").val(),
                   value: value ? 1 : 0},
           });
    return false;
  });
}

function bulk_numbering() {
  close_modal("#bulk_modal");
  $("#bulk_details_title").text("Bulk Numbering");
  $("#who_label").text("Assign car numbers to");
  $("#bulk_details div.hidable").addClass("hidden");
  $("#numbering_controls").removeClass("hidden");

  show_secondary_modal("#bulk_details_modal", function(event) {
    close_secondary_modal("#bulk_details_modal");
    $.ajax(g_action_url,
           {type: 'POST',
            data: {action: 'racer.bulk',
                   what: 'number',
                   who: $("#bulk_who").val(),
                   start: $("#bulk_numbering_start").val(),
                   renumber: $("#renumber").is(':checked') ? 1 : 0},
           });
                  
    return false;
  });
}
  
function bulk_eligibility() {
  close_modal("#bulk_modal");
  $("#bulk_details_title").text("Bulk Eligibility");
  $("#who_label").text("Change eligibility for");
  $("#bulk_details div.hidable").addClass("hidden");
  $("#elibility_controls").removeClass("hidden");

  show_secondary_modal("#bulk_details_modal", function(event) {
    close_secondary_modal("#bulk_details_modal");
    $.ajax(g_action_url,
           {type: 'POST',
            data: {action: 'racer.bulk',
                   what: 'eligibility',
                   who: $("#bulk_who").val(),
                   value: $("#bulk_eligible").is(':checked') ? 1 : 0},
           });
                  
    return false;
  });
}

function disable_preview(msg) {
  var preview = $("#preview").html('<h2>Webcam Disabled</h2>')
    .css({'border': '2px solid black',
          'background': '#d2d2d2'});
  $("<p></p>").text(msg).css({'font-size': '18px'}).appendTo(preview);

  if (window.location.protocol == 'http:') {
    var https_url = "https://" + window.location.hostname + window.location.pathname;
    $("<p>You may need to switch to <a href='" +  https_url + "'>" + https_url + "</a></p>")
      .css({'font-size': '18px'})
      .appendTo(preview);
  }
}

// In (some versions of) Safari, if Flash isn't enabled, the Webcam instance
// just silently fails to load.  We want to present a modal to the user in that
// case, so make clear what the issue is.
function arm_webcam_dialog() {
  var loaded = false;
  Webcam.on('load', function() { loaded = true; });
  // If the Webcam instance is going to present an error, we don't want to put
  // up another one.
  Webcam.on('error', function(msg) {
    loaded = true;
    disable_preview(msg);
  });
  setTimeout(function() {
    if (!loaded) {
      disable_preview('You may have to enable Flash, or give permission to use your webcam.');
    }
  }, 5000);
}

// For #photo_drop form:
Dropzone.options.photoDrop = {
  paramName: 'photo',
  maxFiles: 1,
  maxFilesize: 8,
  url: 'action.php',
  acceptedFiles: 'image/*',
  // dropzone considers the upload successful as long as there was an HTTP response.  We need to look at the
  // message that came back and determine whether the file was actually accepted.
  sending: function(xhr, form_data) {
    preserve_autocrop_state();
  },
  success: function(file, response) {
    this.removeFile(file);

    var data = $.parseXML(response);
    var photo_url_element = data.getElementsByTagName('photo-url');
    if (photo_url_element.length > 0) {
      $("#photo-" + $("#photo_modal_racerid").val() + " img[data-repo='" + $("#photo_modal_repo").val() + "']").attr(
        'src', photo_url_element[0].childNodes[0].nodeValue);
    }

    close_modal("#photo_modal");
  },
};

function setup_webcam() {
  var settings = {
	  width: g_width,
	  height: g_height,
	  dest_width: g_width,
	  dest_height: g_height,
	  crop_width: g_width,
	  crop_height: g_height,
  };
  if (g_cameraIndex < g_cameras.length && g_cameras[g_cameraIndex]) {
	settings['constraints'] = {
	  deviceId: {exact: g_cameras[g_cameraIndex]}
	};
  }
  Webcam.set(settings);
}

window.addEventListener('orientationchange', function() {
  if (screen.width < screen.height) {
    g_width = 480;
    g_height = 640;
  } else {
    g_width = 640;
    g_height = 480;
  }

  Webcam.reset();
  setup_webcam();
  Webcam.attach('#preview');
});

// ***********************
// Original definition, minus the enumerate_cameras call.  On modern browsers,
// this gets redefined, below, to use ES6-only features.
function show_photo_modal(racerid, repo) {
  var firstname = $('#firstname-' + racerid).text();
  var lastname = $('#lastname-' + racerid).text();
  $("#racer_photo_name").text(firstname + ' ' + lastname);
  $("#racer_photo_repo").text(repo);
  $("#photo_modal_repo").val(repo);
  $("#photo_modal_racerid").val(racerid);

  set_autocrop_state(repo);

  // If the racer's already been checked in, don't offer "Capture & Check In" button
  $("#capture_and_check_in").toggleClass('hidden', $("#passed-" + racerid).prop('checked'));

  // TODO Two different submit buttons that set a global, g_check_in.  Eww.
  show_modal("#photo_modal", function() {
    preserve_autocrop_state();
    take_snapshot(racerid, repo, lastname + '-' + firstname);
      return false;
  });

  if (screen.width < screen.height) {
    g_width = 480;
    g_height = 640;
  }

  arm_webcam_dialog();

  Webcam.reset();
  setup_webcam();
  Webcam.attach('#preview');
}

function show_racer_photo_modal(racerid) {
  show_photo_modal(racerid, 'head');
}
function show_car_photo_modal(racerid) {
  show_photo_modal(racerid, 'car');
}

function take_snapshot(racerid, repo, photo_base_name) {
  if (photo_base_name.length <= 1) {
    photo_base_name = 'photo';
  }

  // g_check_in set by onclick method in submit buttons
  if (g_check_in) {
    $("#passed-" + racerid).prop('checked', true);
    $("#passed-" + racerid).trigger("change", true);

    $.ajax(g_action_url,
           {type: 'POST',
            data: {action: 'racer.pass',
                   racer: racerid,
                   value: 1},
           });
  }

  Webcam.snap(function(data_uri) {
	// detect image format from within image_data_uri
	var image_fmt = '';
	if (data_uri.match(/^data\:image\/(\w+)/))
	  image_fmt = RegExp.$1;
	else
	  throw "Cannot locate image format in Data URI";

	// extract raw base64 data from Data URI
	var raw_image_data = data_uri.replace(/^data\:image\/\w+\;base64\,/, '');

	// create a blob and decode our base64 to binary
	var blob = new Blob( [ Webcam.base64DecToArr(raw_image_data) ], {type: 'image/'+image_fmt} );

	// stuff into a form, so servers can easily receive it as a standard file upload
	var form_data = new FormData();
	form_data.append('action', 'photo.upload');
    form_data.append('racerid', racerid);
    form_data.append('repo', repo);
    // image_fmt.replace is for jpeg -> jpg
	form_data.append('photo', blob, photo_base_name + "." + image_fmt.replace(/e/, ''));
    if ($("#autocrop").prop('checked')) {
      form_data.append('autocrop', '1');
    }

    // Testing for <failure> elements occurs in dashboard-ajax.js
    $.ajax(g_action_url,
           {type: 'POST',
            data: form_data,
            contentType: false,
            processData: false,
            success: function(data) {
              var photo_url_element = data.getElementsByTagName('photo-url');
              if (photo_url_element.length > 0) {
                $("#photo-" + racerid + " img[data-repo='" + repo + "']").attr(
                  'src', photo_url_element[0].childNodes[0].nodeValue);
              }
            }
           });

    Webcam.reset();
    close_modal("#photo_modal");
  });
}

function close_photo_modal() {
  Webcam.reset();
  close_modal("#photo_modal");
}

function compare_first(a, b) {
  for (var i = 0; i < a[0].length; ++i) {
    if (a[0][i] < b[0][i]) return -1;
    if (a[0][i] > b[0][i]) return 1;
  }
  return 0;
}

function handle_sorting_event(event) {
  g_order = $(event.target).attr('data-order');
  $("thead a[data-order]").prop('href', '#');
  $(event.target).removeAttr('href');
  console.log("Setting g_order = " + g_order);  // TODO
  sort_checkin_table();
  return false;
}

function sorting_key(row) {
  if (g_order == 'class') {
    // rankseq, lastname, firstname
    return [parseInt(row.querySelector('[data-rankseq]').getAttribute('data-rankseq')),
            row.getElementsByClassName('sort-lastname')[0].innerHTML,
            row.getElementsByClassName('sort-firstname')[0].innerHTML]
  } else if (g_order == 'car') {
      // carnumber (numeric), lastname, firstname
      return [parseInt(row.getElementsByClassName('sort-car-number')[0].innerHTML),
              row.getElementsByClassName('sort-lastname')[0].innerHTML,
              row.getElementsByClassName('sort-firstname')[0].innerHTML]
  } else /* 'name' */ {
      // lastname, firstname
      return [row.getElementsByClassName('sort-lastname')[0].innerHTML,
              row.getElementsByClassName('sort-firstname')[0].innerHTML]
  }
}

function sort_checkin_table() {
	row_array = [];
    rows = $(".main_table tbody").get(0).getElementsByTagName('tr');

	for (var j = 0; j < rows.length; ++j) {
	    row_array[row_array.length] = [sorting_key(rows[j]), rows[j]];
	}

	row_array.sort(compare_first);

	tb = $(".main_table tbody").get(0);

	for (var j = 0; j < row_array.length; ++j) {
        row_array[j][1].classList.remove('d' + (j & 1));
        row_array[j][1].classList.add('d' + ((j + 1) & 1));
	    tb.appendChild(row_array[j][1]);
	}

	delete row_array;
}

// g_action_on_barcode is set in checkin.php with a value persisted in the PHP
// session for this user.
$(function() {
  $("input[name='barcode-handling']").prop('checked', false);
  $("input[name='barcode-handling'][value='" + g_action_on_barcode + "']").prop('checked', 'checked');
  mobile_radio_refresh($("#barcode_settings_modal input[type=radio]"));

  $("#barcode_settings_modal input[type=radio]")
    .on('change', on_barcode_handling_change);
});
function handle_barcode_button_click() {
  show_modal("#barcode_settings_modal", function(event) {
    close_modal("#barcode_settings_modal");
    console.log($("#barcode_settings_modal input[type=radio]:checked"));
    return false;
  });
}
function on_barcode_handling_change() {
  g_action_on_barcode = $("input[name='barcode-handling']:checked").val();
  // Update the session to persist this choice
  $.ajax(g_action_url,
         {type: 'POST',
          data: {action: 'session.write',
                 'session_barcode-action': g_action_on_barcode}
         });

  return false;
}

function global_keypress(event) {
    if ($(":focus").length == 0) {
        $(document).off("keypress");  // We want future keypresses to go to the search form
        $("#find-racer-text").focus();
    }
}

function remove_search_highlighting() {
    $("span.found-racer").each(function() {
        var p = $(this).parent();
        $(this).contents().unwrap();
        p.get()[0].normalize();
    });
}

function cancel_find_racer() {
    $("#find-racer-text").val("");
    $("#find-racer").removeClass("notfound");
    $("#find-racer-index").data("index", 1).text(1);
    $("#find-racer-count").text(0);
    $("#find-racer-message").css({visibility: 'hidden'});
    // TODO $("#find-racer").addClass("hidden");
    remove_search_highlighting();
    $(document).on("keypress", global_keypress);
}

function scroll_and_flash_row(row) {
  $("html, body").animate({scrollTop: row.offset().top - $(window).height() / 2}, 250);
  row.addClass('highlight');
  setTimeout(function() {
    row.removeClass('highlight');
    $("#find-racer-text").val("");
  }, 250);
  setTimeout(function() {
    row.addClass('highlight');
  }, 500);
  setTimeout(function() {
    row.removeClass('highlight');
  }, 750);

  $("#find-racer-index").data("index", 1).text(1);
  $("#find-racer-count").text(0);
  $("#find-racer-message").css({visibility: 'hidden'});
  $("#find-racer").removeClass("notfound");
}

// Returns true if processed as a barcode scan
function maybe_barcode(raw_search) {
  if (raw_search.startsWith('PWDid') && raw_search.length == 8) {
    remove_search_highlighting();
    var row = $("tr[data-racerid=" + parseInt(raw_search.substr(5)) + "]");
  } else if (raw_search.startsWith('PWD') && raw_search.length == 6) {
    remove_search_highlighting();
    var cell = $("td[data-car-number=" + parseInt(raw_search.substr(3)) + "]");
    var row = cell.closest('tr');
  } else {
    return false;
  }

  if (row.length != 1) {
    return false;
  }
  
  scroll_and_flash_row(row);
  console.log(g_action_on_barcode);

  var racerid = row.attr('data-racerid');
  if (g_action_on_barcode == "locate") {
  } else if (g_action_on_barcode == "checkin") {
    var cb = $("#passed-" + racerid);

    setTimeout(function() {
      cb.prop('checked', true);
      // This will update the flipswitch and post the check-in.
      cb.change();
    }, 750);
  } else {  // racer-photo, car-photo
    var repo = g_action_on_barcode == "racer-photo" ? "head" : "car";
    setTimeout(function() {
      console.log('show_photo_modal ' + racerid + ', ' + repo);
      show_photo_modal(racerid, repo);
    }, 750);
  }

  return true;
}

// In response to each onchange event for the #find-racer-text control, hide the
// table rows that don't contain the value string.
function find_racer() {
  var raw_search = $("#find-racer-text").val();
  if (maybe_barcode(raw_search)) {
    return;
  }

  var search_string = raw_search.toLowerCase();
  if (search_string.length == 0) {
    cancel_find_racer();
  } else {
    var domain = $("#main_checkin_table tbody tr")
        .find("td.sort-firstname, td.sort-lastname, td.sort-car-number");
    var find_count = domain.filter(function() {
      // this = <td> element for firstname, lastname, or car number
      return $(this).text().toLowerCase().indexOf(search_string) != -1;
    }).length;
    if (find_count != 0) {
      $("#find-racer").removeClass("notfound");
      remove_search_highlighting();
      domain.contents().each(function() {
        if (this.nodeType === 3) {  // Node.TEXT_NODE Text node
          var where = $(this).text().toLowerCase().indexOf(search_string);
          if (where != -1) {
            var match = this.splitText(where);
            match.splitText(search_string.length);
            $(match).wrap('<span class="found-racer"></span>');
          }
        }
      });
      $("#find-racer-index").data("index", 1).text(1);
      $("#find-racer-count").text(find_count);
      $("#find-racer-message").css({visibility: 'visible'});
      // Scroll the first selection to the middle of the window
      $("html, body").animate({scrollTop: $("span.found-racer").offset().top - $(window).height() / 2}, 250);
    } else {
      console.log("No match!");
      $("#find-racer").addClass("notfound");
      $("#find-racer-index").data("index", 1).text(1);
      $("#find-racer-count").text(0);
      $("#find-racer-message").css({visibility: 'hidden'});
    }
  }
}

function scroll_to_nth_found_racer(n) {
    var found = $("span.found-racer").eq(n - 1);
    $("html, body").animate({scrollTop: found.offset().top - $(window).height() / 2}, 250);
}

// inc = 1 for next found racer, -1 for previous
function next_or_previous_found_racer(inc) {
  var count = $("#find-racer-index").data("index");
  if (inc > 0 && count < $("span.found-racer").length) {
    ++count;
  } else if (inc < 0 && count > 1) {
    --count;
  } else {
    return;
  }

  $("#find-racer-index").data("index", count).text(count);
  scroll_to_nth_found_racer(count);
}

function intercept_arrow_key(event) {
  switch (event.which) {
  case 38:  // up
    next_or_previous_found_racer(-1);
    event.preventDefault();
    break;
  case 40: // down
    next_or_previous_found_racer(+1);
    event.preventDefault();
    break;
  case 9:  // tab or shift-tab
    next_or_previous_found_racer(event.shiftKey ? -1 : +1);
    event.preventDefault();
    break;
  case 27:  // esc
    cancel_find_racer();
    event.preventDefault();
    break;
  }
}

$(function() {
    $(document).on("keypress", global_keypress);
    $("#find-racer-text").on("input", find_racer)
                         .on("keydown", intercept_arrow_key);
    // jquery mobile would add a distracting "blue glow" around the input form
    // after the text input receives focus.  Ugh.
  $("#find-racer-text").off('focus');

  $("thead a[data-order]").on('click', handle_sorting_event);
});
