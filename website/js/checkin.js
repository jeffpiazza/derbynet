// Requires dashboard-ajax.js
// Requires modal.js

// Maps partitionid to highest existing carnumber for that partitionid
g_next_carnumbers = [];
g_poll_max_interval = 0;
function poll_max_carnumbers() {
  $.ajax(g_action_url,
         {type: 'GET',
          data: {query: 'poll',
                 values: 'car-numbers'},
          success: function (data) {
            if (data["cease"]) {
              clearInterval(g_poll_max_interval);
              window.location.href = '../index.php';
              return;
            }
            if (data.hasOwnProperty('car-numbers')) {
              read_next_carnumbers(data['car-numbers']);
            }
          }
         });
}

// carnos is an array of {partitionid, next_carnumber}
function read_next_carnumbers(carnos) {
  for (var i = 0; i < carnos.length; ++i) {
    g_next_carnumbers[parseInt(carnos[i].partitionid)] =
      carnos[i].next_carnumber;
  }
}
function next_carnumber(partitionid) {
  return g_next_carnumbers[partitionid] || 999;
}
$(function() {
  g_poll_max_interval = setInterval(poll_max_carnumbers, 10000);
  poll_max_carnumbers();

  $("#edit_partition").on('change', function(event) {
    // The car number field is updatable only for new racers
    if ($("#edit_carno").attr('data-updatable')) {
      var p = parseInt($("#edit_partition").val());
      if (p && p >= 0) {
        $("#edit_carno").val(next_carnumber(p));
      }
    }
  });
});

// var g_order specified in checkin.php

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
          data: {action: 'award.xbs',
                 racer: racer,
                 value: value},
         });
}

// Two <select> controls for choosing partitions (namely, the one in the
// #edit_racer_modal and the one in the #bulk_who for car numbering) have an
// "(Edit partitions)" entry for manipulating the list of partitions.
function on_edit_partition_change(select, partitions_modal) {
  var select = $(select);
  if (select.val() < 0) {
    // Close any secondary or tertiary modals, which would be in front of partitions_modal.
    while (g_modal_dialogs.length > 1) {
      pop_modal();
    }
    push_modal(partitions_modal);
    select.val(select.prop('last-selected')).trigger('change');
  } else {
    select.prop('last-selected', select.val());
  }
}

function callback_after_partition_modal(op, arg) {
  console.log('callback_after_partition_modal', op, arg);
  if (op == 'add') {  // arg = {partitionid, name}
    poll_max_carnumbers();
    var opt = $("<option/>")
      .attr('value', arg.partitionid)
        .text(arg.name);
    opt.appendTo("#edit_partition");
    opt.clone().appendTo("#bulk_who");
    // Delete "Default" partition, if any, after creating a real partition
    $("#edit_partition option[value=0]").remove();
    $("#bulk_who option[value=0]").remove();
    divid = -1;
    // Move the "Edit Partitions" option to the end and select the new partition
    $("#edit_partition").append($("#edit_partition option[value=" + divid + "]"))
      .val(arg.partitionid).trigger('change');
    mobile_select_refresh("#edit_partition");
    $("#bulk_who").append($("#bulk_who option[value=" + divid + "]"))
      .val(arg.partitionid);
    mobile_select_refresh("#bulk_who");
  } else if (op == 'delete') {  // arg = {partitionid}
    $("#edit_partition option[value=" + arg.partitionid + "]").remove();
    $("#bulk_who option[value=" + arg.partitionid + "]").remove();
  } else if (op == 'rename') {  // arg = {partitionid, name}
    $("#edit_partition option[value=" + arg.partitionid + "]").text(arg.name);
    $("#bulk_who option[value=" + arg.partitionid + "]").text(arg.name);
    // Change the name for every racer in that partition
    $("#main_tbody td[data-partitionid=" + arg.partitionid + "]").text(arg.name);
  } else if (op == 'reorder') { // arg = array of partitionid
    for (var i = 0; i < arg.length; ++i) {
      var divid = arg[i];
      // Move the existing elements around
      $("#edit_partition").append($("#edit_partition option[value=" + divid + "]"));
      $("#bulk_who").append($("#bulk_who option[value=" + divid + "]"));
    }
    // Move the "Edit Partitions" option to the end
    divid = -1;
    $("#edit_partition").append($("#edit_partition option[value=" + divid + "]"));
    $("#bulk_who").append($("#bulk_who option[value=" + divid + "]"));
  }
}

function show_edit_racer_form(racerid) {
  var first_name = $('#firstname-' + racerid).text();
  var last_name = $('#lastname-' + racerid).text();
  var car_no = $('#car-number-' + racerid).text();
  var car_name = $('#car-name-' + racerid).text();

  var class_name = $('#class-' + racerid).text();
  var rank_name = $('#rank-' + racerid).text();
  var note_from = $('#note-from-' + racerid).text();

  $("#edit_racer").val(racerid);

  $("#edit_firstname").val(first_name);
  $("#edit_lastname").val(last_name);

  $("#edit_carno").removeAttr('data-updatable').val(car_no);
  $("#edit_carname").val(car_name);
  $("#edit_note_from").val(note_from);

  var partitionid = $('#div-' + racerid).attr('data-partitionid');
  $("#edit_partition").val(partitionid).prop('last-selected', partitionid);
  $("#edit_partition").change();

  $("#eligible").prop("checked",
                      parseInt($('#lastname-' + racerid).attr("data-exclude")) == 0);
  $("#eligible").trigger("change", true);

  $("#delete_racer_extension").removeClass('hidden');

  show_modal("#edit_racer_modal", "#edit_firstname", function(event) {
    handle_edit_racer();
    return false;
  });
}

function show_new_racer_form() {
  $("#edit_racer").val(-1);

  $("#edit_firstname").val("");
  $("#edit_firstname").focus();

  $("#edit_lastname").val("");

  var partitionid = $("#edit_partition option").first().attr('value');
  
  $("#edit_carno").attr('data-updatable', '1').val(next_carnumber(partitionid));
  $("#edit_carname").val("");
  $("#edit_note_from").val("");

  $("#edit_partition").val(partitionid).prop('last-selected', partitionid);
  $("#edit_partition").change();

  $("#eligible").prop("checked", true);
  $("#eligible").trigger("change", true);

  $("#delete_racer_extension").addClass('hidden');
  
  show_modal("#edit_racer_modal", "#edit_firstname", function(event) {
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
  var new_note_from = $("#edit_note_from").val().trim();

  var new_div_id = $("#edit_partition").val();
  var new_div_name = $('[value="' + new_div_id + '"]', $("#edit_partition")).text();

  var exclude = $("#eligible").is(':checked') ? 0 : 1;

  $.ajax(g_action_url,
         {type: 'POST',
          data: {action: racerid >= 0 ? 'racer.edit' : 'racer.add',
                 racer: racerid,
                 firstname: new_firstname,
                 lastname: new_lastname,
                 carno: new_carno,
                 carname: new_carname,
                 note_from: new_note_from,
                 partitionid: new_div_id,
                 exclude: exclude},
          success: function(data) {
            if (new_div_id <= 0) {
              // Default partition id, used for the first racer (only); load the real partition id
              location.reload(true);
            }
            if (data.hasOwnProperty('warnings')) {
              window.alert("WARNING: " + data.warnings[0]);
            }
            if (data.hasOwnProperty('car-numbers')) {
              read_next_carnumbers(data['car-numbers']);
            }
            if (data.hasOwnProperty('new-row')) {
              var row = addrow0(data['new-row']);
              flipswitch(row.find('input[type="checkbox"].flipswitch'));
              setTimeout(function() { scroll_and_flash_row(row); }, 100);
            } else {
              console.log('Changing partition to ' + new_div_name);
              $("#div-" + racerid)
                .attr('data-partitionid', new_div_id)
                .attr('data-div-sortorder', data['partition-sortorder'])
                .text(new_div_name);
              $("#lastname-" + racerid)
                .attr("data-exclude", exclude ? 1 : 0)
                .text(new_lastname)
                .parents('tr').toggleClass('exclude', exclude == 1);
              $("#firstname-" + racerid).text(new_firstname);
              $("#car-number-" + racerid).attr('data-car-number', new_carno).text(new_carno);
              $("#car-name-" + racerid).text(new_carname);
              $("#note-from-" + racerid).text(new_note_from);
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

function bulk_who_value() {
  if ($("#bulk_who").val() == 'all') {
    return 'all';
  }
  return 'd' + $("#bulk_who").val();
}

function bulk_check_in(value) {
  close_modal_leave_background("#bulk_modal");
  $("#bulk_details_title").text(value ? "Bulk Check-In" : "Bulk Check-In Undo");
  $("#who_label").text(value ? "Check in racers in" : "Undo check-in of racers in");
  $("#bulk_details div.hidable").addClass("hidden");

  show_modal("#bulk_details_modal", function(event) {
    close_modal("#bulk_details_modal");
    $.ajax(g_action_url,
           {type: 'POST',
            data: {action: 'racer.bulk',
                   what: 'checkin',
                   who: bulk_who_value(),
                   value: value ? 1 : 0},
           });
    return false;
  });
}


function on_bulk_numbering_change() {
  $("#bulk_numbering_start").prop('disabled', $("#number_auto").is(':checked'));
  if (!$("#number_auto").is(':checked')) {
    $("#numbering_start_div").slideDown(500);
    $("#bulk_numbering_explanation").slideUp(500);
    $("#bulk_numbering_start").focus();
  } else {
    $("#numbering_start_div").slideUp(500);
    $("#bulk_numbering_explanation").slideDown(500);
  }
}
$(function() {
  $("#number_auto").on('change', on_bulk_numbering_change);
});

function bulk_numbering() {
  close_modal_leave_background("#bulk_modal");
  $("#bulk_details_title").text("Bulk Renumbering");
  $("#who_label").text("Assign car numbers to");
  $("#bulk_details div.hidable").addClass("hidden");
  $("#numbering_controls").removeClass("hidden");

  show_modal("#bulk_details_modal", function(event) {
    close_modal("#bulk_details_modal");
    $.ajax(g_action_url,
           {type: 'POST',
            data: {action: 'racer.bulk',
                   what: 'number',
                   who: bulk_who_value(),
                   auto: $("#number_auto").is(':checked') ? 1 : 0,
                   start: $("#bulk_numbering_start").val(),
                  }
           });
    
    return false;
  });
}

function bulk_eligibility() {
  close_modal_leave_background("#bulk_modal");
  $("#bulk_details_title").text("Bulk Eligibility");
  $("#who_label").text("Change eligibility for");
  $("#bulk_details div.hidable").addClass("hidden");
  $("#elibility_controls").removeClass("hidden");

  show_modal("#bulk_details_modal", function(event) {
    close_modal("#bulk_details_modal");
    $.ajax(g_action_url,
           {type: 'POST',
            data: {action: 'racer.bulk',
                   what: 'eligibility',
                   who: bulk_who_value(),
                   value: $("#bulk_eligible").is(':checked') ? 1 : 0},
           });
    
    return false;
  });
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
  sort_checkin_table();
  return false;
}

function sorting_key(row) {
  if (g_order == 'partition') {
    // partition sortorder, lastname, firstname
    return [parseInt(row.querySelector('[data-div-sortorder]').getAttribute('data-div-sortorder')),
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
  rows = $("#main_tbody").get(0).getElementsByTagName('tr');

  for (var j = 0; j < rows.length; ++j) {
	row_array[row_array.length] = [sorting_key(rows[j]), rows[j]];
  }

  row_array.sort(compare_first);

  tb = $("#main_tbody").get(0);

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

function update_qrcode() {
  $("#mobile-checkin-qrcode").empty();
  $("#mobile-checkin-title").text($("#mobile-checkin-url").val());
  new QRCode(document.getElementById('mobile-checkin-qrcode'),
             {text: $("#mobile-checkin-url").val(),
              width: 256,
              height: 256});
}
function handle_qrcode_button_click() {
  update_qrcode();
  show_modal("#qrcode_settings_modal");
}
function on_mobile_checkin_submit() {
  console.log('on_mobile_checkin_submit');
  update_qrcode();
  return false;
}
$(function() {
  $("#mobile-checkin-url").val(g_preferred_urls[0] + "/mcheckin.php");
  $("#mobile-checkin-form").on('submit', on_mobile_checkin_submit);
});

function global_keypress(event) {
  if (document.activeElement == document.body ||
      document.activeElement == null) {
    // If no other element holds focus, focus on the search box.  This is
    // especially important with barcode scanners, as a lack of focus will
    // ignore the scanned text.
    //
    // We're invoked on a keypress event; the actual input key will be read as
    // part of handling for the keyup that follows.
    $("#find-racer-text").focus();
    console.log('Focusing on search box');
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
}

function scroll_and_flash_row(row) {
  scroll_to_row(row);

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
    var domain = $("#main-checkin-table tbody tr")
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
      scroll_to_nth_found_racer(1);
    } else {
      console.log("No match!");
      $("#find-racer").addClass("notfound");
      $("#find-racer-index").data("index", 1).text(1);
      $("#find-racer-count").text(0);
      $("#find-racer-message").css({visibility: 'hidden'});
    }
  }
}

function scroll_to_row(row) {  // row is a jquery for one tr element
  var div = $("#main-checkin-table-div");
  var th_height = $("#main-checkin-table th").eq(0).closest('tr').height();
  // delta is the number of pixels from the top of the table to the middle of the row
  var delta = row.offset().top + row.height() / 2 - $("#main-checkin-table").offset().top;

  // pixels from the top of the div that vertically centers the tr, excluding the th row
  var goal = (div.height() - th_height) / 2 + th_height;

  // scrollTop + goal = delta, so scrollTop = delta - goal

  var padding = 10;  // 10 pixels vertical padding on tbody
  $("#main-checkin-table-div").animate({scrollTop: delta - goal + padding});
}

function scroll_to_nth_found_racer(n) {
  scroll_to_row($("span.found-racer").eq(n - 1).closest('tr'));
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


// TODO We might be in a better position to know the row number (and parity)
// than the server (which sends rowno).
function make_table_row(racer, xbs) {
  var tr = $('<tr/>').attr('data-racerid', racer.racerid)
      .addClass('d' + (racer.rowno & 1))
      .toggleClass('den_scheduled', racer.denscheduled)
      .toggleClass('exclude', racer.exclude);
  tr.append($('<td>')
            .append('<input type="button" class="white-button" value="Change"' +
                    ' onclick="show_edit_racer_form(' + racer.racerid + ')"/>'));

  tr.append($('<td>')
            .attr('id', 'div-' + racer.racerid)
            .attr('data-partitionid', racer.partitionid)
            .attr('data-div-sortorder', racer.partition_sortorder)
            .text(racer.partition));

  tr.append($('<td class="sort-car-number"/>')
            .attr('data-car-number', racer.carnumber)
            .attr('id', 'car-number-' + racer.racerid)
            .text(racer.carnumber));

  tr.append($('<td/>').attr('id', 'photo-' + racer.racerid)
            .append($('<a href="javascript:show_racer_photo_modal(' + racer.racerid + ')"/>')
                    .append($('<img class="checkin-photo" data-repo="head"/>')
                            .attr('src', racer.headshot)))
            .append($('<a href="javascript:show_car_photo_modal(' + racer.racerid + ')"/>')
                    .append($('<img class="checkin-photo" data-repo="car"/>')
                            .attr('src', racer.carphoto))));

  tr.append($('<td class="sort-lastname"/>')
            .attr('id', 'lastname-' + racer.racerid)
            .attr('data-exclude', racer.exclude ? 1 : 0)
            .text(racer.lastname));
  tr.append($('<td class="sort-firstname"/>')
            .attr('id', 'firstname-' + racer.racerid)
            .text(racer.firstname));
  tr.append($('<td/>')
            .append($("<div/>")
                    .attr('id', 'car-name-' + racer.racerid)
                    .addClass('carname')
                    .text(racer.carname))
            .append($("<div/>")
                    .attr('id', 'note-from-' + racer.racerid)
                    .text(racer.note)));

  var checkin = $('<td class="checkin-status"/>').appendTo(tr);
  checkin.append('<br/>');
  checkin.append($('<input type="checkbox" class="flipswitch"/>')
                 .attr('id', 'passed-' + racer.racerid)
                 .attr('name', 'passed-' + racer.racerid)
                 .prop('checked', racer.passed)
                 .attr('data-on-text', 'Yes')
                 .attr('data-off-text', 'No')
                 // prop onchange doesn't seem to allow a string, but attr does
                 .attr('onchange', 'handlechange_passed(this, ' +
                       JSON.stringify(racer.firstname + ' ' + racer.lastname) +
                       ')'));
  if (racer.scheduled) {
    if (racer.passed) {
      checkin.append(' Racing!');
    } else {
      checkin.append(' Scheduled but not passed');
    }
  } else if (racer.denscheduled) {
    // denscheduled means their racing group has a schedule
    checkin.append(' Late!');
  }

  if (xbs) {
    tr.append($('<td/>')
              .append($('<label/>')
                      .attr('for', 'xbs-' + racer.racerid)
                      .text(xbs + '?'))
              .append('<br/>')
              .append($('<input type="checkbox" class="flipswitch"/>')
                      .attr('name', 'xbs-' + racer.racerid)
                      .prop('checked', racer.xbs)
                      .attr('data-on-text', 'Yes')
                      .attr('data-off-text', 'No')
                      .attr('onchange', 'handlechange_xbs(this);')));
  }

  return tr;
}

function add_table_row(tbody, racer, xbs) {
  return make_table_row(racer, xbs).appendTo($(tbody));
}
