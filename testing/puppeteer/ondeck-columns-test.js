const assert = require('./assert.js');

const puppeteer = require('puppeteer');

var root = 'http://localhost-will-fail/derbynet';
if (process.argv.length > 2) {
  root = process.argv[2];
}
if (root.substring(0, 'http://'.length) != 'http://') {
  root = 'http://' + root;
}
if (root.substring(root.length - 1) == '/') {
  root = root.substring(0, root.length - 1);
}

var debugging = false;

// Test that all the rows of the on-deck table have the same number of columns.
// ("Byes" can make this a little tricky.)

puppeteer.launch({devtools: debugging}).then(async browser => {
  const all_pages = await browser.pages();
  const page = all_pages[0];

  var get_column_counts = function () {
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
  };
  
  await page.goto(root + '/ondeck.php');
  var widths = await page.evaluate(get_column_counts);
  widths.forEach(function(v) {
    assert.equal(v, widths[0]);
  });

  await page.goto(root + '/racer-results.php');
  widths = await page.evaluate(get_column_counts);
  widths.forEach(function(v) {
    assert.equal(v, widths[0]);
  });

  if (!debugging) {
    await browser.close();
  }
});
