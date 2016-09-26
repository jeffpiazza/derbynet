
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
    uploadTableRowsFrom(header_row_present() ? 2 : 1, 'award.import');
  });
});
