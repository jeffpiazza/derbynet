function print_selected() {
  var doc_class = $("input[name='doc-class']:checked").val();
  var doc_class_details = doc_classes[doc_class];

  // ids is either the selected racerids or the selected awardids
  var ids = [];

  if (doc_class_details['type'] == 'racer') {
    $("input[data-racerid]:checked").each(function(index, checkbox) {
      ids.push($(checkbox).attr('data-racerid'));
    });
  } else if (doc_class_details['type'] == 'award') {
    $("input[data-awardid]:checked").each(function(index, checkbox) {
      ids.push($(checkbox).attr('data-awardid'));
    });
  }
  if (ids.length == 0) {
    // Button should have been disabled anyway
    return;
  }

  var options = {};
  $.each(doc_class_details['options'], function(opt, opt_data) {
    var control = $("input[name^='" + doc_class + "-" + opt + "']");
    if (opt_data.type == 'bool') {
      options[opt] = control.is(':checked');
    } else if (opt_data.type == 'string') {
      options[opt] = control.val();
    } else {
      console.error('Unrecognized option type: ' + opt_data.type);
    }
  });
  
  window.open("render-document.php/" + doc_class_details['type'] + "/" + doc_class
              + "?options=" + encodeURIComponent(JSON.stringify(options))
              + "&ids=" + ids.join(),
              "_blank");
}

function update_print_button() {
  var doc_class = $("input[name='doc-class']:checked").val();
  var doc_class_details = doc_classes[doc_class];
  var data_prop;
  if (doc_class_details['type'] == 'racer') {
    data_prop = 'data-racerid';
  } else if (doc_class_details['type'] == 'award') {
    data_prop = 'data-awardid';
  }
  var enabled = $("input[" + data_prop + "]:checked").length > 0;
  $("#print-selected").prop('disabled', !enabled);
}

// Clicking on a row in the table of racers: treat as a click on the checkbox
// for that row.
function on_tr_click(event) {
  if ($(event.target).is(":not(:checkbox)")) {
    $(this).find("input:checkbox").click();
  }
}

function process_racer_list(data) {
  var racers = data.getElementsByTagName('racer');
  var table = $("div#subject-racers table");
  $.each(racers, function (index, racer) {
    var racerid = racer.getAttribute("racerid");
    var rows = table.find("tr");
    var tr;
    if (rows.length > index &&
        rows.slice(index, index + 1).find("input[data-racerid='" + racerid + "']").length > 0) {
      tr = rows.slice(index, index + 1);
    } else {
      tr = $("<tr></tr>");
      if (rows.length <= index) {
        tr.appendTo(table);
      } else {
        tr.insertBefore(rows[index]);
      }
      $("<td><input type='checkbox' data-racerid='" + racerid + "'/></td>").appendTo(tr)
        .on('change', function() { update_print_button(); });
      $("<td></td>").appendTo(tr);  // Car number
      $("<td></td>").appendTo(tr);  // Racer name
      $("<td class='photo-column'></td>").appendTo(tr).append("<img></img>");
      tr.on("click", on_tr_click);
    }
    var cells = tr.find("td");
    $(cells[1]).text(racer.getAttribute("carnumber"));
    $(cells[2]).text(racer.getAttribute("firstname") + " " + racer.getAttribute("lastname"));
    $(cells[3]).find("img").prop("src", racer.getAttribute("headshot"));
  });

  // Chop off any extra rows, but first preserve checkbox settings for any moved racers
  var condemned = table.find("tr").slice(racers.length);
  condemned.find("input:checkbox").each(function() {
    $("input:checkbox[data-racerid='" + $(this).attr('data-racerid') + "']")
      .prop('checked', $(this).prop('checked'));
  });
  condemned.remove();
}

function process_award_list(data) {
  var awards = data.getElementsByTagName('award');
  var table = $("div#subject-awards table");
  $.each(awards, function(index, award) {
    var awardid = award.getAttribute('awardid');
    var rows = table.find("tr");
    var tr;
    if (rows.length > index &&
        rows.slice(index, index + 1).find("input[data-awardid='" + awardid + "']").length > 0) {
      tr = rows.slice(index, index + 2);
    } else {
      tr = $("<tr></tr>");
      if (rows.length <= index) {
        tr.appendTo(table);
      } else {
        tr.insertBefore(rows[index]);
      }
      $("<td><input type='checkbox' data-awardid='" + awardid + "'/></td>").appendTo(tr)
        .on('change', function() { update_print_button(); });
      $("<td></td>").appendTo(tr);  // Award Name
      $("<td></td>").appendTo(tr);  // Recipient
    }
    var cells = tr.find("td");
    $(cells[1]).text(award.getAttribute('awardname'));
    $(cells[2]).text(award.getAttribute('firstname') + ' ' + award.getAttribute('lastname'));
  });
}

function handle_sortorder_racers_change() {
  // This means there's a delay between when the sort order is changed and the
  // display actually reflects that.
  $.ajax("action.php",
         {type: 'GET',
          data: {query: "racer.list",
                 order: $("#sortorder-racers option").val()},
          success: function(data) {
            process_racer_list(data);
          },
         });
}

function handle_sortorder_awards_change() {
  /*
  $.ajax("action.php",
         {type: 'GET',
          data: {query: "racer.list",
                 order: $("#sortorder").val()},
          success: function(data) {
            process_racer_list(data);
          },
         });
*/
}

function poll() {
  $.ajax("action.php",
         {type: 'GET',
          data: {query: "racer.list",
                 order: $("#sortorder-racers option:selected").val()},
          success: function(data) {
            process_racer_list(data);
          },
         });
  $.ajax("action.php",
         {type: 'GET',
          data: {query: "award.list",
                 adhoc: 1},
          success: function(data) {
            process_award_list(data);
          },
         });
  update_print_button();
}

function select_all(checked) {
  var doc_class = $("input[name='doc-class']:checked").val();
  var doc_class_details = doc_classes[doc_class];
  var data_prop;
  if (doc_class_details['type'] == 'racer') {
    data_prop = 'data-racerid';
  } else if (doc_class_details['type'] == 'award') {
    data_prop = 'data-awardid';
  }
  
  $("input[type=checkbox][" + data_prop + "]").prop("checked", checked);
  update_print_button();
}

// Also modifies what sortorder options are displayed, and what subjects list is
// shown.
function reveal_doc_specific_options() {
  $("div[data-docname]").addClass("hidden");

  var doc_class = $("input[name='doc-class']:checked").val();
  var doc_class_details = doc_classes[doc_class];
  $("div[data-docname='" + doc_class + "']").removeClass("hidden");

  // Switch between award/racer selections, depending on type of docclass
  if (doc_class_details['type'] == 'racer') {
    $("#subject-racers, #sortorder-racers-div").removeClass("hidden");
    $("#subject-awards, #sortorder-awards-div").addClass("hidden");
  } else if (doc_class_details['type'] == 'award') {
    $("#subject-racers, #sortorder-racers-div").addClass("hidden");
    $("#subject-awards, #sortorder-awards-div").removeClass("hidden");
  }
}

$(function() {
  poll();
  setInterval(function() { poll(); }, 10000);
  reveal_doc_specific_options();
  $("input[type=radio][name='doc-class']").change(function() { reveal_doc_specific_options(); });
});
