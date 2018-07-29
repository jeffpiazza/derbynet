function print_selected() {
  var ids = [];

  $("input[data-racerid]:checked").each(function(index, checkbox) {
    ids.push($(checkbox).attr('data-racerid'));
  });

  if (ids.length == 0) {
    // Button should have been disabled anyway
    return;
  }

  var doc_class = $("input[name='doc-class']:checked").val();

  var options = {};
  $.each(doc_classes[doc_class], function(opt, opt_data) {
    var control = $("input[name^='" + doc_class + "-" + opt + "']");
    if (opt_data.type == 'bool') {
      options[opt] = control.is(':checked');
    } else if (opt_data.type == 'string') {
      options[opt] = control.val();
    } else {
      console.error('Unrecognized option type: ' + opt_data.type);
    }
  });
  
  window.open("render-document.php/racer/" + doc_class
              + "?options=" + encodeURIComponent(JSON.stringify(options))
              + "&ids=" + ids.join(),
              "_blank");
}

function update_print_button() {
  var enabled = $("input[data-racerid]:checked").length > 0;
  $("#print-selected").prop('disabled', !enabled);
}

function on_tr_click(event) {
  if ($(event.target).is(":not(:checkbox)")) {
    $(this).find("input:checkbox").click();
  }
}

function process_racer_list(data) {
  var racers = data.getElementsByTagName('racer');
  var table = $("div#racers table");
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

function handle_sortorder_change() {
  $.ajax("action.php",
         {type: 'GET',
          data: {query: "racer.list",
                 order: $("#sortorder").val()},
          success: function(data) {
            process_racer_list(data);
          },
         });
}

function poll() {
  $.ajax("action.php",
         {type: 'GET',
          data: {query: "racer.list",
                 order: $("#sortorder option:selected").val()},
          success: function(data) {
            process_racer_list(data);
          },
         });
  update_print_button();
}

function select_all(checked) {
  $("input[data-racerid]").prop("checked", checked);
}

function reveal_doc_specific_options() {
  $("div[data-docname]").addClass("hidden");
  var doc_class = $("input[name='doc-class']:checked").val();
  $("div[data-docname='" + doc_class + "']").removeClass("hidden");
}

$(function() {
  poll();
  setInterval(function() { poll(); }, 10000);
  reveal_doc_specific_options();
  $("input[type=radio][name='doc-class']").change(function() { reveal_doc_specific_options(); });
});
