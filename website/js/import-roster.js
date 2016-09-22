
// Redefines onDrop from import-csv.js
function onDrop(draggable, droppable) {
  if (draggable.attr('data-field') == 'classname') {
    collectClassNames(droppable.attr('data-column'));
  }
}

// TODO: Something similar for ranks/subgroups
// TODO: action.php?query=class.list, and compare existing classes
function collectClassNames(columnNumber) {
  $('#new_ranks').empty();
  $('#new_ranks').append('<h4>' + $('[data-field="classname"]:first').text() + 's</h4>');
  var classnames = [];
  $('td.column' + columnNumber).each(function(index, elt) {
    // Don't count a header_row value
    if ($(elt).closest('.header_row').length == 0) {
      classnames[$(elt).text()] = 1;
    }
  });
  for (var cl in classnames) {
    $('#new_ranks').append(document.createTextNode(cl), '<br/>');
  }
}

$(function() {
  if (window.File && window.FileReader) {
    $('#csv_file').bind('change', onFileSelect);
  } else {
    $('#meta').html('<h3>Please update your browser</h3>'
                    + '<p>Operation of this page depends on HTML5 features for local file handling' 
                    + ' that this browser doesn\'t support.  Nearly all browsers support these'
                    + ' features in their most recent versions.</p>');
  }

  $('#import_button').click(function() {
    // First row of file is labeled data-row="1".
    // If the file contains a header row, the first data to process is row 2, otherwise row 1.
    uploadTableRowsFrom(header_row_present() ? 2 : 1, 'racer.import');
  });
});
