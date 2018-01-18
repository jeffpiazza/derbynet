// These functions are available to be redefined by individual pages:
//

// Tells whether the content table should include a row of droppable label targets
function usingLabelTargets() {
  return true;
}

// onFileContentLoaded fires a short while after the file has finished loading.
function onFileContentLoaded(file) {
}

// onDrop fires when a label is dragged to a th.label_target, but not when it's
// dragged back to the palette of labels.
function onDrop(draggable, droppable) {
}
// onUndrop fires when a label is dragged back to the palette
function onUndrop(draggable) {
}

/////////////////////////////////////////////////////////////////////////////////////////
// Populate the page's main table with the data from the csv.
/////////////////////////////////////////////////////////////////////////////////////////
function onFileSelect(event) {
  var file = event.target.files[0];
  
  loadCsvContentTable(file, $('input:radio[name="encoding-select"]:checked').val(),
                      usingLabelTargets());

  $(".file_target").addClass("hidden");
  $("#encoding").addClass("hidden");

  $(".fields").removeClass("hidden");
  $("#import_button_div").removeClass("hidden");

  enableOrDisableImportButton();

  // The .label_target elements are only created by printTable(), above, and
  // don't appear immediately within the DOM.  Delaying like this seems to work,
  // but isn't efficient or guaranteed reliable.
  setTimeout(function() {
    onFileContentLoaded(file);
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
          var target_row = $('<tr id="label_target_row"/>').appendTo(csv_content);
          target_row.append('<th/>');
          for (var item in data[row]) {
            $('<th class="label_target"><p>(Drag Label Here)</p></th>').attr("data-column", item)
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

function enableOrDisableImportButton() {
  if ($('.field.required').closest('[data-home]').length == 0) {
    $('#import_button_div input[type="button"]').prop('disabled', false);
    $('#assign_label_message').addClass('hidden');
  } else {
    $('#import_button_div input[type="button"]').prop('disabled', true);
    $('#assign_label_message').removeClass('hidden');
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

      enableOrDisableImportButton();

      onDrop($(ui.draggable[0]), $(this));
    }
  });
}

/////////////////////////////////////////////////////////////////////////////////////////
// Perform the actual data upload
/////////////////////////////////////////////////////////////////////////////////////////

// Returns an array mapping zero-based column numbers to the assigned parameter
// name, based on field labels dragged to label targets.
function collectDraggedLabels() {
  var targets = $('#csv_content').find('.label_target');
  var parameter_names = new Array(targets.length);
  targets.each(function (index, label_target) {
    // A label dragged into place is presented as a
    // td.label_target containing a div.field
    var field = $(label_target).find('.field');
    // Might be empty if there was an extra column left unlabeled.
    if (field.length > 0) {
      parameter_names[$(label_target).attr('data-column')] = field.attr('data-field');
    }
  });
  return parameter_names;
}

// 'row' is the numeric row index (1-based) for the next row to upload;
// 'action' is the ajax action which will receive the upload.
//
// action is the name of the ajax action, e.g., 'roster.import'.
//
// parameter_names is an array mapping column number to parameter name for the
// action.  Typically parameter_names is computed by collectDraggedLabels,
// above.
function uploadTableRowsFrom(row, action, parameter_names) {
  if ($('[data-row="' + row + '"]').length == 0) {
    onUploadComplete();
    return;
  }

  // Marshall the cell content from the row into an array of params for upload
  var params = {action: action};
  var allblank = true;
  for (var col = 0; col < parameter_names.length; ++col) {
    // Might be empty if there was an extra column left unlabeled.
    if (parameter_names[col]) {
      var txt = $('#csv_content [data-row="' + row + '"] .column' + col).text();
      params[parameter_names[col]] = txt;
      if (txt.trim().length > 0) {
        allblank = false;
      }
    }
  }

  if (allblank) {
    $('[data-row="' + row + '"] th').text('SKIP');
    uploadTableRowsFrom(row + 1, action, parameter_names);
  } else {
    $.ajax(g_action_url,
           {type: 'POST',
            data: params,
            global: false,  // Be sure that a failure doesn't present a modal alert
            success: function(xmldoc) {
              var ok = xmldoc.documentElement.getElementsByTagName("success");
              if (ok && ok.length > 0) {
                $('[data-row="' + row + '"] th').append('<span class="ok_outcome">OK</span>');
              } else {
                $('[data-row="' + row + '"] th').append('<span class="failed_outcome">FAILED </span>');
                var fail = xmldoc.documentElement.getElementsByTagName("failure");
                if (fail && fail.length > 0) {
                  $('[data-row="' + row + '"] th').append(fail[0].childNodes[0].nodeValue);
                }
              }
              uploadTableRowsFrom(row + 1, action, parameter_names);
            },
            error: function(jqXHR, textStatus, errorThrown) {
              alert("Error: " + textStatus + " / " + errorThrown);
            }
           });
  }
}

function uploadTableRowsWithParameters(action, parameter_names) {
    // First row of file is labeled data-row="1".
    // If the file contains a header row, the first data to process is row 2, otherwise row 1.
  uploadTableRowsFrom(header_row_present() ? 2 : 1, action, parameter_names);
}  

function uploadTableRows(action) {
  uploadTableRowsWithParameters(action, collectDraggedLabels());
}

function onUploadComplete() {
  $(".fields").addClass('hidden');
  $('#import_button_div').addClass('hidden');

  $("#assign_label_message").text("Import successful!")
    .removeClass('hidden')
    .css('color', '#008000');
}

/////////////////////////////////////////////////////////////////////////////////////////
// Document ready
/////////////////////////////////////////////////////////////////////////////////////////
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

      enableOrDisableImportButton();

      onUndrop($(ui.draggable[0]));
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
    // Event is received by the <input> element, whose parent is the
    // .file_target div
    $(event.target).parent().addClass("draghover");
  });
  $(".file_target input").on('dragleave', function() {
    $(event.target).parent().removeClass("draghover");
  });

  if (window.File && window.FileReader) {
    $('#csv_file').bind('change', onFileSelect);
  } else {
    $('#meta').html('<h3>Please update your browser</h3>'
                    + '<p>Operation of this page depends on HTML5 features for local file handling' 
                    + ' that this browser doesn\'t support.  Nearly all browsers support these'
                    + ' features in their most recent versions.</p>');
  }
});
