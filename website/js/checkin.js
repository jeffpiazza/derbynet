// Requires dashboard-ajax.js
// Requires modal.js

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
  $("#autocrop").flipswitch("refresh");
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
    // cb is the checkbox element, with name "pased-" plus the racer id, e.g., passed-1234
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
            data: {action: 'award.xbs',
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

  show_modal("#edit_racer_modal", function(event) {
      handle_edit_racer();
      return false;
  });
}

function handle_edit_racer() {
  close_modal("#edit_racer_modal");

  var racerid = $("#edit_racer").val();

  var new_firstname = $("#edit_firstname").val();
  var new_lastname = $("#edit_lastname").val();
  var new_carno = $("#edit_carno").val();
  var new_carname = $("#edit_carname").val();

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

function disable_preview(msg) {
  $("#preview").html('<h2>Webcam Disabled</h2>')
    .append('<p></p>')
    .css({'border': '2px solid black',
          'background': '#d2d2d2'});
  $("#preview p").text(msg).css({'font-size': '18px'});
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
  }, 2000);
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


function switch_camera_modal() {
  g_cameraIndex++;
  if ( g_cameraIndex >= g_cameras.length ) {
    g_cameraIndex = 0;
  }

  Webcam.reset();
  Webcam.set({
	  width: g_width,
	  height: g_height,
	  dest_width: g_width,
	  dest_height: g_height,
	  crop_width: g_width,
	  crop_height: g_height,
	  constraints: {
		  deviceId: g_cameras[g_cameraIndex]
	  }
  });
  Webcam.attach('#preview');
}

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

  if( screen.width < screen.height ) {
    g_width = 480;
    g_height = 640;
  }

  var i = 0;

  navigator.mediaDevices.enumerateDevices()
  .then(function(devices) {
    devices.forEach(function(device) {
      if( device.kind=== "videoinput") {
        g_cameras[i]= device.deviceId;
        i++;
      }
    });
  });

  arm_webcam_dialog();
  Webcam.set({
	  width: g_width,
	  height: g_height,
	  dest_width: g_width,
	  dest_height: g_height,
	  crop_width: g_width,
	  crop_height: g_height,
	  constraints: {
		  deviceId: g_cameras[g_cameraIndex]
	  }
  });
  Webcam.attach('#preview');
}

window.addEventListener('orientationchange', function() {
  if( screen.width < screen.height ) {
    g_width = 480;
    g_height = 640;
  } else {
    g_width = 640;
    g_height = 480;
  }

  Webcam.reset();
  Webcam.set({
	  width: g_width,
	  height: g_height,
	  dest_width: g_width,
	  dest_height: g_height,
	  crop_width: g_width,
	  crop_height: g_height,
	  constraints: {
		  deviceId: g_cameras[g_cameraIndex]
	  }
  });
  Webcam.attach('#preview');
});

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
	  form_data.append('photo', blob, photo_base_name + "."+image_fmt.replace(/e/, '') );
      if ($("#autocrop").prop('checked')) {
        form_data.append('autocrop', '1');
      }

      // Testing for <failure> elements occurs in dashboard-ajax.js
      $.ajax(g_action_url,
             {type: 'POST',
              data: form_data,
              cache: false,
              contentType: false,
              processData: false,
              success: function(data) {
                  var photo_url_element = data.getElementsByTagName('photo-url');
                  if (photo_url_element.length > 0) {
                      $("#photo-" + racerid + " img[data-repo='" + repo + "'").attr(
                          'src', photo_url_element[0].childNodes[0].nodeValue);
                  }
              }
             });

      close_modal("#photo_modal");
  });
}

function close_photo_modal() {
    close_modal("#photo_modal");
}

function compare_first(a, b) {
  if (a[0] == b[0])
      return 0;
  if (a[0] < b[0])
      return -1;
   return 1;
}


function sorting_key(row) {
  if (g_order == 'class') {
      // class, lastname, firstname
      return [row.getElementsByClassName('sort-class')[0].innerHTML,
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

// In response to each onchange event for the #find-racer-text control, hide the
// table rows that don't contain the value string.
function find_racer() {
    var search_string = $("#find-racer-text").val().toLowerCase();
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

function intercept_arrow_key(event) {
    switch (event.which) {
    case 38:  // up
        {
            var count = $("#find-racer-index").data("index");
            if (count > 1) {
                --count;
                $("#find-racer-index").data("index", count).text(count);
                scroll_to_nth_found_racer(count);
                event.preventDefault();
                return;
            }
        }
        break;
    case 40: // down
        {
            var count = $("#find-racer-index").data("index");
            if (count < $("span.found-racer").length) {
                ++count;
                $("#find-racer-index").data("index", count).text(count);
                scroll_to_nth_found_racer(count);
                event.preventDefault();
                return;
            }
        }
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
});
