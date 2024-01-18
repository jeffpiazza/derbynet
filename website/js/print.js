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
  } else if (doc_class_details['type'] == 'summary') {
    // ids aren't meaningful for this, but we need at least one.
    ids = [0];
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
    } else if (opt_data.type == 'int') {
      options[opt] = parseInt(control.val());
    } else if (opt_data.type == 'string') {
      options[opt] = control.val();
    } else if (opt_data.type == 'radio') {
      options[opt] = control.filter(':checked').val();
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
  var enabled;
  if (doc_class_details['type'] == 'racer') {
    enabled = $("input[data-racerid]:checked").length > 0;
  } else if (doc_class_details['type'] == 'award') {
    enabled = $("input[data-awardid]:checked").length > 0;
  } else if (doc_class_details['type'] == 'summary') {
    enabled = true;
  }
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
  var table = $("div#subject-racers table");
  $.each(data.racers, function (index, racer) {
    var racerid = racer.racerid;
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
      $("<td class='photo-column'></td>").appendTo(tr)
        .append($("<img/>").css('max-height', 80));
      tr.on("click", on_tr_click);
    }
    var cells = tr.find("td");
    $(cells[1]).text(racer.carnumber);
    $(cells[2]).text(racer.firstname + " " + racer.lastname);
    $(cells[3]).find("img").prop("src", racer.headshot);
  });

  // Chop off any extra rows, but first preserve checkbox settings for any moved racers
  var condemned = table.find("tr").slice(data.racers.length);
  condemned.find("input:checkbox").each(function() {
    $("input:checkbox[data-racerid='" + $(this).attr('data-racerid') + "']")
      .prop('checked', $(this).prop('checked'));
  });
  condemned.remove();
}

// Returns a table mapping awardtypeid's (as strings) to awardtype names
function make_award_types(data) {
  var awardtypes = {};
  $.each(data['award-types'], function(index, awardtype) {
    awardtypes[awardtype.awardtypeid] = awardtype.awardtype;
  });
  return awardtypes;
}

// Returns a table mapping:
//   'c' + classid -> class name
//   'r' + rankid -> rank name
function make_classes_and_ranks(data) {
  var classes_and_ranks = {};
  $.each(data.classes, function(index, cl) {
    classes_and_ranks['c' + cl.classid] = cl.name;
    $.each(cl.subgroups, function(index, rank) {
      classes_and_ranks['r' + rank.rankid] = rank.name;
    });
  });
  return classes_and_ranks;
}

function process_award_list(data) {
  var awardtypes = make_award_types(data);
  var classes_and_ranks = make_classes_and_ranks(data);
  var table = $("div#subject-awards table");
  $.each(data.awards, function(index, award) {
    var awardid = award.awardid;
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
      $("<td>" +
        "<p class='awardname'></p>" +
        "<p class='awardtype'></p>" +
        "</td>").appendTo(tr);
      $("<td>" +
        "<p class='classname'></p>" +
        // TODO rankname
        "</td>").appendTo(tr);  // Award Type, Class, Rank
      $("<td></td>").appendTo(tr);  // Recipient
    }
    var cells = tr.find("td");
    tr.find(".awardname").text(award.awardname);
    tr.find(".awardtype").text(awardtypes[award.awardtypeid]);
    tr.find(".classname").text(classes_and_ranks['c' + award.classid]);
    // TODO ranks
    $(cells[3]).text(award.firstname + ' ' + award.lastname);
  });
}

function handle_sortorder_racers_change() {
  // This means there's a delay between when the sort order is changed and the
  // display actually reflects that.
  $.ajax("action.php",
         {type: 'GET',
          data: {query: "racer.list",
                 order: $("#sortorder-racers option:selected").val()},
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

  $("#subject-racers, #subject-awards, #subject-summary").addClass('hidden');
  $("#sortorder-racers-div, #sortorder-awards-div, #sortorder-summary-div").addClass('hidden');
  
  var doc_class = $("input[name='doc-class']:checked").val();
  var doc_class_details = doc_classes[doc_class];
  $("div[data-docname='" + doc_class + "']").removeClass("hidden");

  // Switch between award/racer selections, depending on type of docclass
  if (doc_class_details['type'] == 'racer') {
    $("#subject-racers, #sortorder-racers-div").removeClass("hidden");
  } else if (doc_class_details['type'] == 'award') {
    $("#subject-awards, #sortorder-awards-div").removeClass("hidden");
  } else if (doc_class_details['type'] == 'summary') {
    $("#subject-summary, #sortorder-summary-div").removeClass("hidden");
  }
  $("#page-controls").toggleClass('hidden', doc_class_details['type'] == 'summary');
  update_print_button();
}

$(function() {
  poll();
  setInterval(function() { poll(); }, 10000);
  reveal_doc_specific_options();
  $("input[type=radio][name='doc-class']").change(function() { reveal_doc_specific_options(); });
});
