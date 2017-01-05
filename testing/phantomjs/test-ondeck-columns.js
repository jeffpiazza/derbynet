require('./preamble');

// Test that all the rows of the on-deck table have the same number of columns.
// ("Byes" can make this a little tricky.)

var test_column_widths = function() {
  var result = page.evaluate(function () {
    var results = [];
    // For the racer-results page, there may be at most 1 rowspan > 1 cell in a
    // row; carry_forward is the number of rows before the extra rowspan is
    // accounted for.
    var carry_forward = 0;
    $('tr').each(function() {
      var col_count = carry_forward;
      if (carry_forward > 0) {
        --carry_forward;
      }
      $(this).find('td,th').each(function () {
        if ($(this).attr('rowspan')) {
          carry_forward = $(this).attr('rowspan') - 1;
        }
        if ($(this).attr('colspan')) {
          col_count += +$(this).attr('colspan');
        } else {
          col_count++;
        }
      });
      results.push(col_count);
    });
    return results;
  });

  result.forEach(function(v) {
    assert.equal(v, result[0]);
  });
  console.log("  " + "  " + "  " + scriptName + ": " + result.length +
              " table row(s) for " + page.url);
};

page.open(root + 'ondeck.php', function(status) {
  assert.success(status);
  test_column_widths(page);
  page.open(root + 'racer-results.php', function(status) {
    assert.success(status);
    test_column_widths(page);
    phantom.exit(0);
  });
});

