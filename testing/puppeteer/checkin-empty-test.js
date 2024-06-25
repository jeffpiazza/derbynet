// Assumes running against an empty database, to confirm behviors creating new
// partitions and racers.

const assert = require('./assert.js');

const puppeteer = require('puppeteer');

var root = 'http://localhost/derbynet';
if (process.argv.length > 2) {
  root = process.argv[2];
}
if (root.substring(0, 'http://'.length) != 'http://') {
  root = 'http://' + root;
}
if (root.substring(root.length - 1) == '/') {
  root = root.substring(0, root.length - 1);
}

var debugging = false

var role = 'RaceCoordinator';
var password = 'doyourbest';

puppeteer.launch({devtools: debugging}).then(async browser => {
  const all_pages = await browser.pages();
  const page = all_pages[0];
  page.setViewport({width: 1200,
                    height: 1800});
  if (debugging) {
    page.on('console', msg => { console.log('REMOTE: ' + msg._text); });
  }

  await page.goto(root + '/login.php');
  await page.evaluate(function (role, password) {
    window.handle_login(role, password);
  }, role, password);
  await page.waitForNavigation();

  await page.evaluateOnNewDocument(() => {
    window.phantom_testing = true;
  });

  await page.goto(root + '/checkin.php');

  // Confirm table is empty.  If it's not, reset the database before running this test.
  assert.equal(0, await page.$$eval("#main_tbody tr", trs => { return trs.length; }));

  // Click "New Racer" button, to open the new racer dialog
  await page.evaluate(() => { $("input[value='New Racer']").click(); });
  // Click #edit_partition select, and pick the "Edit Partitions" option
  await page.select('#edit_partition', '-1');

  // No dens/partitions appear in the listview that allows ordering
  assert.equal(0, await page.evaluate(() => {
    return $(".partition_modal ul").filter(":visible").find("li").length; }));
  // #166: Trying to create a first partition/den with an empty name
  await page.evaluate(() => { $(".partition_modal input[value='New Den']").filter(":visible").click(); });
  await page.evaluate(() => { $(".partition_naming_modal input[type=submit]").filter(":visible").click(); });

  // No new dens/partitions appear in the listview
  assert.equal(0, await page.evaluate(() => {
    return $(".partition_modal ul").filter(":visible").find("li").length; }));
  await page.evaluate(() => { $(".partition_modal input[value='Close']").filter(":visible").click(); });

  // After rejected "add partition", try again: Click "New Racer" button, to
  // open the new racer dialog
  await page.evaluate(() => { $("input[value='New Racer']").click(); });
  assert.equal(0, await page.evaluate(() => { return $("#edit_partition").val(); }));

  // Create a first racer.
  await page.evaluate(() => {
    $("#edit_firstname").val("Fred");
    $("#edit_lastname").val("Flintstone");
    $("#edit_carno").val('');
    $("#edit_racer_modal input[type=submit]").filter(":visible").click(); });
  // Upon creating a first racer, the page reloads (because of the likelihood
  // there's a new partition instantiated as a result).
  await page.waitForNavigation();
  // One racer now present
  assert.equal(1, await page.$$eval("#main_tbody tr", trs => { return trs.length; }));
  // Car number "101"
  assert.equal(101, await page.$$eval("#main_tbody tr td.sort-car-number",
                                      cno => { return $(cno).text(); }));
  assert.equal("Default", await page.$$eval("#main_tbody tr #div-1",
                                            part => { return $(part).text(); }));

  // Rename the Default partition by clicking on "Change" button for the one racer
  await page.evaluate(() => { $("#main_tbody input[value='Change']").filter(":visible").click(); });

  // Selected partition has id 1, as it's a real partition, not a manufactured
  // "Default" entry with value 0.
  assert.equal(1, await page.evaluate(() => { return $("#edit_partition").val(); }));
  await page.select('#edit_partition', '-1');
  assert.equal(1, await page.evaluate(() => {
    return $(".partition_modal ul").filter(":visible").find("li").length; }));
  assert.equal("Default (1)", await page.evaluate(() => {
    return $(".partition_modal ul li p").filter(":visible").text(); }));
  await page.evaluate(() => { $(".partition_modal ul li a").filter(":visible").click(); });
  await page.evaluate(() => {
    $(".partition_naming_modal input[type=text]").filter(":visible").val("Unique Partition");
    $(".partition_naming_modal input[type=submit]").filter(":visible").click();
  });
  await page.waitForSelector(".partition_naming_modal", {hidden: true});

  await page.waitForFunction(() => {
    return $(".partition_modal ul li p").filter(":visible").text() == "Unique Partition (1)";
  });
  assert.equal(1, await page.evaluate(() => {
    return $(".partition_modal ul").filter(":visible").find("li").length; }));
  assert.equal("Unique Partition (1)", await page.evaluate(() => {
    return $(".partition_modal ul li p").filter(":visible").text(); }));
  await page.evaluate(() => { $(".partition_modal input[value='Close']").click(); });

  assert.equal("Unique Partition", await page.$$eval("#main_tbody tr #div-1",
                                                     part => { return $(part).text(); }));

  if (debugging) {
    console.log('End of test (would close browser if not debugging)');
  } else {
    await browser.close();
  }
});
