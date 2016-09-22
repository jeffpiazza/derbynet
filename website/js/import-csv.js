/////////////////////////////////////////////////////////////////////////////////////////
// Populate the page's main table with the data from the csv.
/////////////////////////////////////////////////////////////////////////////////////////

function onFileSelect(event) {
    var file = event.target.files[0];
    $('#meta').html('<span class="filename">' + file.name + '</span><br/>\n' +
                    'Type: ' + (file.type || '(not available)') + '<br/>\n' +
                    'Size: ' + file.size + ' bytes<br/>\n' +
                    'Last Modified: ' + (file.lastModifiedDate
                                         ? file.lastModifiedDate.toLocaleDateString()
                                         : '(not available)') + '<br/>\n' +
                    'Encoding chosen: ' + $('input:radio[name="encoding-select"]:checked').val());
  loadCsvContentTable(file, $('input:radio[name="encoding-select"]:checked').val(),
                      /* add_label_targets*/true);

    $('#start_over_button').removeClass('hidden');
    $(".fields").removeClass("hidden");
    $(".file_target").addClass("hidden");
    $("#encoding").addClass("hidden");
    $('#submit_message').text('File import will be enabled when required fields are assigned.');

    // TODO: The .label_target elements are only created by
    // printTable(), above, and don't appear immediately within the
    // DOM.  Delaying like this seems to work, but isn't efficient or
    // guaranteed reliable.
    setTimeout(function() {
        makeDroppableLabelTarget($(".label_target"));
    }, 100);
}

// Reads 'file' as text in 'encoding', populating the table named csv_content.
// If 'add_label_targets' is true, then an initial row of label_target <th> elements is
// written to the table.
function loadCsvContentTable(file, encoding, add_label_targets) {
  var reader = new FileReader();

  reader.onload = function(event) {
    var csv = event.target.result;
    var csv_content = $("#csv_content");
    try {
      var data = $.csv.toArrays(csv);
      var html = '';
      var first = true;
      for (var row in data) {
        if (first && add_label_targets) {
          // Add an extra row of label targets on top
          var target_row = $('<tr/>').appendTo(csv_content);
          target_row.append('<th/>');
          for (var item in data[row]) {
            $('<th class="label_target">(Drag Label Here)</th>').attr("data-column", item)
              .appendTo(target_row);
          }
        }
        var table_row = $('<tr/>').attr("data-row", 1 + parseInt(row)).appendTo(csv_content);
        if (first) {
          table_row.addClass("header_row");
        }

        // First column shows the outcome of each row's upload attempt.
        // First row also includes flipswitch for header row.
        if (first) {
          $('<th/>').append(
            '<label for="header-row-present">Header row?</label>',
            '<input type="checkbox" name="header-row-present" id="header-row-present"' +
              ' data-role="flipswitch" checked="checked"/>')
            .appendTo(table_row)
            .trigger("create");
          table_row.find('input[type="checkbox"]').on("change", handleHeaderRowPresentChange);
        } else {
          table_row.append('<th class="outcome"/>');
        }
        for (var item in data[row]) {
          $('<td class="dim"/>').addClass('column' + item).text(data[row][item]).appendTo(table_row);
        }
        first = false;
      }
    } catch(err) {
      alert('Failure: ' + err);
    }
  };

  reader.onerror = function() {
    alert('Unable to read ' + file.fileName);
  };

  reader.readAsText(file, encoding);
}

/////////////////////////////////////////////////////////////////////////////////////////
// Respond to the flipswitch for "header row present?"
/////////////////////////////////////////////////////////////////////////////////////////
function header_row_present() {
  return $("#header-row-present").prop("checked");
}

function handleHeaderRowPresentChange(event) {
    if (header_row_present()) {
        $('[data-row="1"]').addClass("header_row");
    } else {
        $('[data-row="1"]').removeClass("header_row");
    }
}

/////////////////////////////////////////////////////////////////////////////////////////
// Drag and drop behavior for column labels
/////////////////////////////////////////////////////////////////////////////////////////
//
// The relevant page structure is:
//
//  div.fields
//    div.target.ui-droppable
//      table
//        tr
//          td[data-home="<fieldname>"]
//            div.field.ui-draggable.required[data-field="<fieldname>"] -- the draggable field label
//            ...
//  table#csv_content:
//    tr      -- Top row of label targets:
//      th
//      th.label_target.ui-droppable[data-column="0"]
//        -- (div.field moves here if dragged to a column.)
//      th.label_target.ui-droppable[data-column="1"]
//      ...
//    tr[data-row="1"].header_row
//      th
//        (flipswitch control)
//      td.column0.dim "Text of first cell from file"
//      td.column1.dim
//      td.column2.dim
//      ...
//    tr[data-row="2"]
//    ...
//
// The behavior for the .label_target elements is established by calls to
// makeDroppableLabelTarget in onFileSelect, above.

function dragOutOfLabelTarget(label_target_jq) {
  // label_target_jq is the .label_target giving up a .field div
  if (label_target_jq.length == 0) {
    return;
  }
  // Dim all the data cells for the now-unlabeled column
  $('.column' + label_target_jq.attr('data-column')).addClass('dim');
  label_target_jq.removeClass('label_target_filled');

  // No longer filled, the .label_target can once again become droppable.
  makeDroppableLabelTarget(label_target_jq);

  // Calling label_target_jq.text directly will destroy the draggable too
  // soon, so we delay the action.
  setTimeout(function() {
    label_target_jq.text('(Drag Label Here)');
  }, 50);
}

// Expected to be redefined:
function onDrop(draggable, droppable) {
}

function hideOrShowImportButton() {
  if ($('.field.required').closest('[data-home]').length == 0) {
    $('#import_button').removeClass('hidden');
    $('#submit_message').addClass('hidden');
  } else {
    $('#import_button').addClass('hidden');
    $('#submit_message').removeClass('hidden');
  }
}

function makeDroppableLabelTarget(label_target_jq) {
  label_target_jq.droppable({
    hoverClass: 'label_target_hover',
    drop: function (event, ui) {
      // If dragging out of a label_target, make the label_target droppable
      // again
      dragOutOfLabelTarget($(ui.draggable[0]).closest(".label_target"));
      
      // $(this) is the .label_target jquery element.
      // Remove the "(Drop Here)" text and replace with label 
      $(this).text('');
      $(this).append(ui.draggable[0]);

      // With a label in place, we become no longer droppable
      $(this).droppable("destroy");
      $(this).addClass('label_target_filled');
      $('.column' + $(this).attr('data-column')).removeClass('dim');

      hideOrShowImportButton();

      onDrop($(ui.draggable[0]), $(this));
    }
  });
}

$(function() {
  // Establishes a drop target for putting a label back after being incorrectly
  // placed on a column.  
  $('.fields .target').droppable({
    hoverClass: 'fields_target_hover',
    drop: function (event, ui) {
      // event.target is the .target div
      // ui.draggable is the .field

      // If dragging out of a label_target, make the
      // label_target droppable again
      dragOutOfLabelTarget($(ui.draggable[0]).closest(".label_target"));

      // The "find" puts the field back into its "home" cell.
      $(this).find('[data-home="' + $(ui.draggable[0]).attr('data-field') + '"]')
        .append(ui.draggable[0]);

      hideOrShowImportButton();
    }
  });

  $(".field").draggable({
    helper: 'clone',
    appendTo: 'body',
    cursorAt: { top: 0, left: 20 },
    opacity: 0.5,
    revert: 'invalid',
  });

  $(".file_target input").on('dragenter', function() {
    $(event.target).addClass("draghover");
  });
  $(".file_target input").on('dragleave', function() {
    $(event.target).removeClass("draghover");
  });
});

/////////////////////////////////////////////////////////////////////////////////////////
// Perform the actual data upload
/////////////////////////////////////////////////////////////////////////////////////////

// 'row' is the numeric row index (1-based) for the next row to upload;
// 'action' is the ajax action which will receive the upload.
function uploadTableRowsFrom(row, action) {
  if (row < 3) { console.log("uploadTableRowsFrom(" + row + ")"); } // TODO
  if ($('[data-row="' + row + '"]').length == 0) {
    return;
  }

  // Marshall the cell content from the row into an array of params for upload
  var params = {action: action};
  var allblank = true;
  $('#csv_content').find('.label_target').each(function (index, label_target) {
    // A label dragged into place is presented as a
    // td.label_target containing a div.field
    var field = $(label_target).find('.field');
    // Might be empty if there was an extra column left unlabeled.
    if (field.length > 0) {
      var txt =
          $('#csv_content [data-row="' + row + '"]' +
            ' .column' + $(label_target).attr('data-column'))
          .text();
      params[field.attr('data-field')] = txt;
      if (txt.trim().length > 0) {
        allblank = false;
      }
    }
  });

  if (allblank) {
    $('[data-row="' + row + '"] th').text('SKIP');
    uploadTableRowsFrom(row + 1, action);
  } else {
    $.ajax(g_action_url,
           {type: 'POST',
            data: params,
            global: false,  // Be sure that a failure doesn't present a modal alert
            success: function(xmldoc) {
              var ok = xmldoc.documentElement.getElementsByTagName("success");
              if (ok && ok.length > 0) {
                $('[data-row="' + row + '"] th').append('OK');
              } else {
                console.log(xmldoc);
                $('[data-row="' + row + '"] th').append('FAILED');
              }
              uploadTableRowsFrom(row + 1, action);
            },
            error: function(jqXHR, textStatus, errorThrown) {
              alert("Error: " + textStatus + " / " + errorThrown);
            }
           });
  }
}
