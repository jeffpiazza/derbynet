// These functions are available to be redefined by individual pages:
//

// Tells whether the content table should include a row of droppable label targets
function usingLabelTargets() {
  return true;
}

// onFileContentLoaded fires a short while after the file has finished loading.
function onFileContentLoaded(file) {
}

// Fires when the setting on whether there's a header row changes.
function onHeaderRowToggle() {
}

// onDrop fires when a label is dragged to a th.label_target, but not when it's
// dragged back to the palette of labels.
function onDrop(draggable, droppable) {
}
// onUndrop fires when a label is dragged back to the palette
function onUndrop(draggable) {
}

var g_filename;
var g_file_contents;
var g_workbook;

/////////////////////////////////////////////////////////////////////////////////////////
// Populate the page's main table with the data from the csv.
/////////////////////////////////////////////////////////////////////////////////////////
function onFileSelect(event) {
  g_filename = event.target.files[0];

  var reader = new FileReader();

  reader.onload = function(e) {
    g_file_contents = e.target.result;
    loadWorkbook(0);
  };

  reader.onerror = function() {
    alert('Unable to read ' + g_filename.fileName);
  };

  reader.readAsBinaryString(g_filename);
}

// With file already read into g_file_contents
function loadWorkbook(sheet_index) {
  var codepage = $('#select-encoding :selected').val();
  try {
    // For CSV files, XLSX doesn't seem to handle code page choices correctly.
    // Explicitly decoding the file content into a string seems to give better
    // results.
    var translated = cptable.utils.decode(codepage, g_file_contents);
    g_workbook = XLSX.read(translated, {type: 'string'});
  } catch (error) {
    // ... but for xlsx workbooks, the translated string looks like a corrupted
    // zip file, so use the untranslated file content as a binary string.
    g_workbook = XLSX.read(g_file_contents, {type: 'binary',
                                             codepage: codepage});
  }

  if (g_workbook) {
    if (sheet_index >= g_workbook.SheetNames.length) {
      sheet_index = 0;
    }
    buildWorksheetSelector(sheet_index);
    loadContentFromSheet(g_workbook.SheetNames[sheet_index]);
  }
}

function loadContentFromSheet(sheet_name) {
  populateContentTable(sheet_name);

  $(".file_target").addClass("hidden");

  $(".fields").removeClass("hidden");
  $("#import_button_div").removeClass("hidden");

  enableOrDisableImportButton();

  // The .label_target elements are only created by printTable(), above, and
  // don't appear immediately within the DOM.  Delaying like this seems to work,
  // but isn't efficient or guaranteed reliable.
  setTimeout(function() {
    onFileContentLoaded(g_filename);
    makeDroppableLabelTarget($(".label_target"));
  }, 100);
}

function populateContentTable(sheetname) {
  var csv_content = $("#csv_content");
  csv_content.empty();

  var sheet = g_workbook.Sheets[sheetname];

  var default_value = "default-value-hopefully-unlikely-to-occur-in-a-real-spreadsheet";
  var arrays = XLSX.utils.sheet_to_json(sheet, {header: 1, defval: default_value});

  var first = true;
  for (var row in arrays) {
    if (first && usingLabelTargets()) {
      // Add an extra row of label targets on top
      var target_row = $('<tr id="label_target_row"/>').appendTo(csv_content);
      target_row.append('<th/>');
      for (var item in arrays[row]) {
        $('<th class="label_target"><p>(Drag Label Here)</p></th>').attr("data-column", item)
          .appendTo(target_row);
      }
    }
    var table_row = $('<tr/>').attr("data-row", 1 + parseInt(row)).appendTo(csv_content);
    if (first) {
      table_row.addClass("header_row");
      // First column shows the outcome of each row's upload attempt.
      // First row also includes flipswitch for header row.
      $('<th/>').append(
        '<label for="header-row-present">Header row?</label>',
        '<input type="checkbox" name="header-row-present" id="header-row-present"' +
          ' class="flipswitch" checked="checked"/>')
        .appendTo(table_row);
      flipswitch(table_row.find('input'));
      table_row.find('input[type="checkbox"]').on("change", handleHeaderRowPresentChange);
    } else {
      table_row.append('<th class="outcome"/>');
    }
    for (var item in arrays[row]) {
      if (arrays[row][item] == default_value) arrays[row][item] = "";
      $('<td class="dim"/>').addClass('column' + item).text(arrays[row][item]).appendTo(table_row);
    }
    first = false;
  }
}

/////////////////////////////////////////////////////////////////////////////////////////
// Selector for encodings
/////////////////////////////////////////////////////////////////////////////////////////

function buildEncodingSelector() {
  var codePageNames = [
    {codepage: '1252', name: 'Western (Windows-1252, ISO-8859-1)'},
    {codepage: '10000', name: 'Mac OS Roman (Macintosh)'},
    {codepage: '65001', name: 'UTF-8'},
    {},
    {codepage: '437', name: 'DOS Original'},
    {codepage: '850', name: 'DOS Latin-1'},
    {codepage: '895', name: 'DOS Czech'},
    {codepage: '620', name: 'DOS Polish'},
    {codepage: '737', name: 'DOS Greek'},
    {codepage: '852', name: 'DOS Latin-2'},
    {codepage: '857', name: 'DOS Latin-5'},
    {codepage: '861', name: 'DOS Icelandic'},
    {codepage: '865', name: 'DOS Danish/Norwegian'},
    {codepage: '866', name: 'DOS Belarusian/Russian/Ukranian'},
    {codepage: '874', name: 'Windows Thai'},
    {codepage: '932', name: 'Windows Shift-JIS (Windows-932)'},
    {codepage: '936', name: 'Windows Simplified Chinese (Windows-936)'},
    {codepage: '949', name: 'Windows Korean (Windows-949)'},
    {codepage: '950', name: 'Windows Traditional Chinese (Windows-950)'},
    {codepage: '1250', name: 'Windows Central Europe (Windows-1250)'},
    {codepage: '1251', name: 'Windows Cyrillic (Windows-1251)'},
    {codepage: '1253', name: 'Windows Greek (Windows-1253)'},
    {codepage: '1254', name: 'Windows Turkish (Windows-1254)'},
    {codepage: '1255', name: 'Windows Hebrew (Windows-1255)'},
    {codepage: '1256', name: 'Windows Arabic (Windows-1256)'},
    {codepage: '1257', name: 'Windows Baltic (Windows-1257)'},
    {codepage: '1258', name: 'Windows Vietnamese (Windows-1258)'},
    {codepage: '10006', name: 'Macintosh Greek'},
    {codepage: '10007', name: 'Macintosh Cyrillic'},
    {codepage: '10008', name: 'Macintosh Simplified Chinese'},
    {codepage: '10029', name: 'Macintosh Central Europe'},
    {codepage: '10079', name: 'Macintosh Icelandic'},
    {codepage: '10081', name: 'Macintosh Turkish'},
  ];

  var encoding = $("#encoding");
  var select = $("<select id='select-encoding'></select>").prependTo(encoding);
  encoding.prepend("<label for='select-encoding'>Encoding:</label>");

  for (var cpi in codePageNames) {
    var cp = codePageNames[cpi];
    if (cp.hasOwnProperty('codepage')) {
      select.append("<option value=\"" + cp.codepage + "\">" + cp.name + "</option>");
    } else {
      select.append("<option value=\"\" disabled></option>");
    }
  }
  select.val("65001");  // UTF-8
  select.on('change', onEncodingChange);
}

function onEncodingChange() {
  if (g_file_contents) {
    var sheet_index = 0;
    if ($("#select-sheet").length) {
      sheet_index = $("#select-sheet :selected").val();
    }
    loadWorkbook(sheet_index);
  }
}

/////////////////////////////////////////////////////////////////////////////////////////
// Selector for worksheets
/////////////////////////////////////////////////////////////////////////////////////////

function buildWorksheetSelector(sheet_index) {
  var select = $("#select-sheet");
  if (select.length == 0) {
    var sheet = $("#sheet");
    sheet.append("<label for='select-sheet'>Sheet:</label>");
    select = $("<select id='select-sheet'></select>").appendTo(sheet);
    select.off('change');
  }

  select.empty();
  for (var i in g_workbook.SheetNames) {
    var option = $("<option value=\"" + i + "\"/>").appendTo(select);
    option.text(g_workbook.SheetNames[i]);
  }
  select.val(sheet_index);

  select.on('change', onWorksheetChange);
}

function onWorksheetChange() {
  loadContentFromSheet(g_workbook.SheetNames[$("#select-sheet :selected").val()]);
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
  onHeaderRowToggle();
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

      // onDrop may remove some required fields, e.g., for first-last marker, so
      // it's important to call onDrop before enableOrDisableImportButton.
      onDrop($(ui.draggable[0]), $(this));

      enableOrDisableImportButton();
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
//
// failures is the count of row import failures so far
function uploadTableRowsFrom(row, action, parameter_names, failures) {
  if ($('[data-row="' + row + '"]').length == 0) {
    onUploadComplete(failures);
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
    uploadTableRowsFrom(row + 1, action, parameter_names, failures);
  } else {
    $.ajax(g_action_url,
           {type: 'POST',
            data: params,
            global: false,  // Be sure that a failure doesn't present a modal alert
            success: function(data) {
              if (data.outcome.summary == 'success') {
                $('[data-row="' + row + '"] th').append('<span class="ok_outcome">OK</span>');
                if (data.hasOwnProperty('warning')) {
                  $('<span class="warning"></span>')
                    .appendTo('[data-row="' + row + '"] th')
                    .text(data.warning);
                }
              } else {
                ++failures;
                $('[data-row="' + row + '"] th').append('<span class="failed_outcome">FAILED </span>');
                $('[data-row="' + row + '"] th').append(data.outcome.description);
              }
              uploadTableRowsFrom(row + 1, action, parameter_names, failures);
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
  uploadTableRowsFrom(header_row_present() ? 2 : 1, action, parameter_names, 0);
}  

function uploadTableRows(action) {
  uploadTableRowsWithParameters(action, collectDraggedLabels());
}

function onUploadComplete(failures) {
  $(".fields").addClass('hidden');
  $('#import_button_div').addClass('hidden');

  $("#assign_label_message")
    .empty()
    .removeClass('hidden')
    .append("<span class='upload_complete_message'>Import complete!</span>");
  if (failures > 0) {
    $("#assign_label_message")
      .append("<span class='upload_failure_message'> (" + failures + " failed)</span>");
  }
}

/////////////////////////////////////////////////////////////////////////////////////////
// Document ready
/////////////////////////////////////////////////////////////////////////////////////////
$(function() {
  buildEncodingSelector();

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
