const assert = require('./assert.js');

const puppeteer = require('puppeteer');

const fakeAjax = require('./fake-ajax.js');

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

fakeAjax.setDebugging(debugging);

var role = 'RaceCoordinator';
var password = 'doyourbest';

// There's apparently one or more race conditions in the test, but I can't find
// them.  The slowMo makes the test a lot more reliable in the interim.
puppeteer.launch({devtools: debugging, slowMo: 200}).then(async browser => {
  const all_pages = await browser.pages();
  const page = all_pages[0];
  page.setViewport({width: 1200,
                    height: 1800});
  if (debugging) {
    page.on('console', msg => { console.log('REMOTE: ', msg._text); });
  }

  await page.goto(root + '/login.php');
  await page.evaluate(function (role, password) {
    window.handle_login(role, password);
  }, role, password);
  await page.waitForNavigation();

  await page.evaluateOnNewDocument(() => {
    window.phantom_testing = true;
  });

  await page.goto(root + '/coordinator.php');

  await fakeAjax.installOn(page);

  async function open_scheduling_control_group(selector) {
    await page.evaluate((selector) => { $(selector).click(); }, selector);
    
    // Wait for control group to open
    await page.waitForFunction((selector) => {
      console.log(selector + " height = " +  $(selector).height());
      return $(selector).height() > 150;
    },
                               {}, selector);
  }

  async function modal_open(selector) {
    await page.waitForFunction((selector) => {
      return !$(selector).closest(".modal_frame").hasClass('hidden');
    },
                       {}, selector);
    await page.waitForFunction(() => { return $("#modal_background").css('display') == 'block'; });
  }
  async function all_modals_closed() {
    await page.waitForFunction(() => {
      if ($(".modal_frame").not(".hidden").length > 0) {
        console.log("Forcing " + $(".modal_frame").not(".hidden").length + " modal(s) closed.");
        $(".modal_frame").addClass('hidden');
      }
      $(".modal_fram .modal_dialog").css('display', 'none');
      return $(".modal_frame").not(".hidden").length == 0;
    }, 100);
    var v = await page.waitForFunction(() => {
      if ($("#modal_background").css('display') != 'none') {
        // For some reason, the fadeOut in do_close_modal sometimes seems to get starved for
        // processing, causing the waitForFunction to fail.
        console.log("modal_background display = " + $("#modal_background").css('display'));
        $("#modal_background").css('display', 'none');
      }
      return $("#modal_background").css('display') == 'none';
    }, 100);
    console.log('  all_modals_closed success!');
    return v;
  }
  
  // =================================================== First simulated poll =====================================
  await page.evaluate(function(json) { process_coordinator_poll_json(JSON.parse(json)); },
      "{\"current-heat\": {\"now_racing\": true,\n" +
                        "\"use_master_sched\": false,\n" +
                        "\"use_points\": false,\n" +
                      "\"classid\": 3,\n" +
                        "\"roundid\": 3,\n" +
                        "\"round\": 1,\n" +
                        "\"tbodyid\": 3,\n" +
                        "\"heat\": 3,\n" +
                        "\"number-of-heats\": 4,\n" +
                      "\"class\": \"Bears and Frèr\"},\n" +
       "\"heat-results\": [],\n" +
       "\"ready-aggregate\": [],\n" +
       "\"racers\": [{\"lane\": 1,\n" +
                   "\"name\": \"Juan Jacobsen\",\n" +
                   "\"carname\": \"\",\n" +
                   "\"carnumber\": \"343\",\n" +
                   "\"photo\": \"\",\n" +
                   "\"finishtime\": \"\",\n" +
                   "\"finishplace\": \"\"},\n" +
                  "{\"lane\": 2,\n" +
                   "\"name\": \"Jeffress Jamison\",\n" +
                   "\"carname\": \"\",\n" +
                   "\"carnumber\": \"139\",\n" +
                   "\"photo\": \"\",\n" +
                   "\"finishtime\": \"\",\n" +
                   "\"finishplace\": \"\"},\n" +
                  "{\"lane\": 3,\n" +
                   "\"name\": \"Antoine Akiyama\",\n" +
                   "\"carname\": \"\",\n" +
                   "\"carnumber\": \"303\",\n" +
                   "\"photo\": \"\",\n" +
                   "\"finishtime\": \"\",\n" +
                   "\"finishplace\": \"\"}],\n" +
       "\"timer-state\": {\"lanes\": 4,\n" +
                       "\"last_contact\": 1526246081,\n" +
                       "\"state\": 3,\n" +
                       "\"icon\": \"img/status/ok.png\",\n" +
                       "\"message\": \"Staging\"},\n" +
       "\"replay-state\": {\"last_contact\": 0,\n" +
                        "\"state\": 1,\n" +
                        "\"icon\": \"img/status/not_connected.png\",\n" +
                        "\"connected\": false,\n" +
                        "\"message\": \"NOT CONNECTED\"},\n" +
       "\"classes\": [{\"classid\": 1,\n" +
                    "\"count\": 17,\n" +
                    "\"nrounds\": 1,\n" +
                    "\"ntrophies\": -1,\n" +
                    "\"name\": \"Lions & Tigers\",\n" +
                    "\"subgroups\": [{\"rankid\": 1,\n" +
                                   "\"count\": 17,\n" +
                                   "\"name\": \"Lions & Tigers\"}]},\n" +
                   "{\"classid\": 2,\n" +
                    "\"count\": 18,\n" +
                    "\"nrounds\": 1,\n" +
                    "\"ntrophies\": -1,\n" +
                    "\"name\": \"White\'s Wolves\",\n" +
                    "\"subgroups\": [{\"rankid\": 2,\n" +
                                   "\"count\": 18,\n" +
                                   "\"name\": \"White\'s Wolves\"}]},\n" +
                   "{\"classid\": 3,\n" +
                    "\"count\": 17,\n" +
                    "\"nrounds\": 1,\n" +
                    "\"ntrophies\": -1,\n" +
                    "\"name\": \"Bears and Frèr\",\n" +
                    "\"subgroups\": [{\"rankid\": 3,\n" +
                                   "\"count\": 17,\n" +
                                   "\"name\": \"Bears and Frèr\"}]},\n" +
                   "{\"classid\": 4,\n" +
                    "\"count\": 16,\n" +
                    "\"nrounds\": 1,\n" +
                    "\"ntrophies\": -1,\n" +
                    "\"name\": \"Webelos (\\\"Webes\",\n" +
                    "\"subgroups\": [{\"rankid\": 4,\n" +
                                   "\"count\": 16,\n" +
                                   "\"name\": \"Webelos (\\\"Webes\"}]},\n" +
                   "{\"classid\": 5,\n" +
                    "\"count\": 15,\n" +
                    "\"nrounds\": 1,\n" +
                    "\"ntrophies\": -1,\n" +
                    "\"name\": \"Arrows <<--<<\",\n" +
                    "\"subgroups\": [{\"rankid\": 1,\n" +
                                   "\"count\": 15,\n" +
                                   "\"name\": \"Arrows <<--<<\"}]},\n" +
                   "{\"classid\": 7,\n" +
                    "\"count\": 0,\n" +
                    "\"nrounds\": 1,\n" +
                    "\"ntrophies\": -1,\n" +
                    "\"name\": \"TheLastClass\",\n" +
                    "\"subgroups\": [{\"rankid\": 7,\n" +
                                   "\"count\": 0,\n" +
                                   "\"name\": \"TheLastClass\"}]}],\n" +
       "\"rounds\": [{\"roundid\": 1,\n" +
                   "\"classid\": 1,\n" +
                   "\"class\": \"Lions & Tigers\",\n" +
                   "\"round\": 1,\n" +
                   "\"roster_size\": 17,\n" +
                   "\"passed\": 5,\n" +
                   "\"unscheduled\": 0,\n" +
                   "\"heats_scheduled\": 5,\n" +
                   "\"heats_run\": 5,\n" +
                   "\"name\": \"Lions & Tigers, Round 1\"},\n" +
                  "{\"roundid\": 2,\n" +
                   "\"classid\": 2,\n" +
                   "\"class\": \"White\'s Wolves\",\n" +
                   "\"round\": 1,\n" +
                   "\"roster_size\": 18,\n" +
                   "\"passed\": 5,\n" +
                   "\"unscheduled\": 0,\n" +
                   "\"heats_scheduled\": 5,\n" +
                   "\"heats_run\": 5,\n" +
                   "\"name\": \"White\'s Wolves, Round 1\"},\n" +
                  "{\"roundid\": 3,\n" +
                   "\"classid\": 3,\n" +
                   "\"class\": \"Bears and Frèr\",\n" +
                   "\"round\": 1,\n" +
                   "\"roster_size\": 17,\n" +
                   "\"passed\": 3,\n" +
                   "\"unscheduled\": 0,\n" +
                   "\"heats_scheduled\": 4,\n" +
                   "\"heats_run\": 2,\n" +
                   "\"name\": \"Bears and Frèr, Round 1\"},\n" +
                  "{\"roundid\": 4,\n" +
                   "\"classid\": 4,\n" +
                   "\"class\": \"Webelos (\\\"Webes\",\n" +
                   "\"round\": 1,\n" +
                   "\"roster_size\": 15,\n" +
                   "\"passed\": 2,\n" +
                   "\"unscheduled\": 2,\n" +
                   "\"heats_scheduled\": 0,\n" +
                   "\"heats_run\": 0,\n" +
                   "\"name\": \"Webelos (\\\"Webes, Round 1\"},\n" +
                  "{\"roundid\": 5,\n" +
                   "\"classid\": 5,\n" +
                   "\"class\": \"Arrows <<--<<\",\n" +
                   "\"round\": 1,\n" +
                   "\"roster_size\": 16,\n" +
                   "\"passed\": 5,\n" +
                   "\"unscheduled\": 5,\n" +
                   "\"heats_scheduled\": 0,\n" +
                   "\"heats_run\": 0,\n" +
                   "\"name\": \"Arrows <<--<<, Round 1\"},\n" +
                  "{\"roundid\": 7,\n" +
                   "\"classid\": 7,\n" +
                   "\"class\": \"TheLastClass\",\n" +
                   "\"round\": 1,\n" +
                   "\"roster_size\": 0,\n" +
                   "\"passed\": 0,\n" +
                   "\"unscheduled\": 0,\n" +
                   "\"heats_scheduled\": 0,\n" +
                   "\"heats_run\": 0,\n" +
                   "\"name\": \"TheLastClass, Round 1\"}]\n" +
                      "}");

  assert.equal({"now-racing":[3],
                "ready-to-race":[],
                "not-yet-scheduled":[4,5,7],
                "done-racing":[1,2],
               },
               await page.evaluate(() => { return g_rounds_layout; }));

  assert.equal(6, await page.$$eval(".scheduling_control_group .control_group.scheduling_control",
                                    controls => { return controls.length; }));

  assert.equal([[],    // Headers row
                ["1","343","Juan Jacobsen",""],
                ["2","139","Jeffress Jamison",""],
                ["3","303","Antoine Akiyama",""],
                ["4","","",""]],
               await page.evaluate(function() {
                 var table = [];
                 $("#now-racing-group .heat-lineup table").first().find("tr").each(function(index, tr) {
                   var row = [];
                   $(tr).find("td").each(function(index, td) {
                     row.push($(td).text());
                   });
                   table.push(row);
                 });
                 return table;
               }));

  assert.equal("Bears and Frèr, Round 1",
               await page.$eval("#now-racing-group > div > h3", h3 => { return $(h3).text(); }));
  assert.equal("4 heats scheduled, 2 run.",
               await page.$eval("#now-racing-group p", p => { return $(p).text(); }));

  assert.equal([
    // Now racing
    "3", 0,
    // Not yet scheduled
    "4", 1, "Schedule",
    "5", 1, "Schedule",
    "7", 0,  // No passed racers
    // Done racing
    "1", 2, "Make Changes", "Repeat Round",
    "2", 2, "Make Changes", "Repeat Round"],
               await page.evaluate(function() {
                 var buttons = [];
                 $("div.control_group[data-roundid]").each(function() {
                   buttons.push($(this).attr('data-roundid'));
                   buttons.push($(this).find("input[type='button']").length);
                   $(this).find("input[type='button']").each(function() {
                     buttons.push($(this).val());
                   });
                 });
                 return buttons;
               }));

  assert.equal(["Repeat Round", "Add New Rounds"],
               await page.evaluate(function() {
                 var buttons = [];
                 $("#supplemental-control-group input[type='button']").each(function() {
                   buttons.push($(this).val());
                 });
                 return buttons;
               }));

  // ===============================  =====================================

  assert.equal("Heat 3 of 4",
               await page.$eval("#now-racing-group .heat_number h3", h3 => { return $(h3).text(); }));
  
  assert.equal(true, await page.$eval("#is-currently-racing", r => { return $(r).prop('checked'); }));

  await fakeAjax.testForJson(async () => {
    var skip_button = await page.$("#skip_heat_button");
    skip_button.click();
  },
                       {'type': 'POST',
                        'data': {'action': 'heat.select',
                                 'heat': 'next'}},
      "{\"current-heat\": {\"now_racing\": false,\n" +
                        "\"use_master_sched\": false,\n" +
                             "\"use_points\": false,\n" +
                             "\"marker\": 327,\n" +
                        "\"classid\": 3,\n" +
                        "\"roundid\": 3,\n" +
                        "\"round\": 1,\n" +
                        "\"tbodyid\": 3,\n" +
                        "\"heat\": 4,\n" +
                        "\"number-of-heats\": 4,\n" +
                      "\"class\": \"Bears and Frèr\"},\n" +
       "\"heat-results\": [],\n" +
       "\"ready-aggregate\": [],\n" +
       "\"racers\": [{\"lane\": 2,\n" +
                   "\"name\": \"Juan Jacobsen\",\n" +
                   "\"carname\": \"\",\n" +
                   "\"carnumber\": \"343\",\n" +
                   "\"photo\": \"\",\n" +
                   "\"finishtime\": \"\",\n" +
                   "\"finishplace\": \"\"},\n" +
                  "{\"lane\": 3,\n" +
                   "\"name\": \"Jeffress Jamison\",\n" +
                   "\"carname\": \"\",\n" +
                   "\"carnumber\": \"139\",\n" +
                   "\"photo\": \"\",\n" +
                   "\"finishtime\": \"\",\n" +
                   "\"finishplace\": \"\"},\n" +
                  "{\"lane\": 4,\n" +
                   "\"name\": \"Antoine Akiyama\",\n" +
                   "\"carname\": \"\",\n" +
                   "\"carnumber\": \"303\",\n" +
                   "\"photo\": \"\",\n" +
                   "\"finishtime\": \"\",\n" +
                   "\"finishplace\": \"\"}],\n" +
       "\"timer-state\": {\"lanes\": 4,\n" +
                       "\"last_contact\": 1526246081,\n" +
                       "\"state\": 3,\n" +
                       "\"icon\": \"img/status/ok.png\",\n" +
                       "\"message\": \"Staging\"},\n" +
       "\"replay-state\": {\"last_contact\": 0,\n" +
                        "\"state\": 1,\n" +
                        "\"icon\": \"img/status/not_connected.png\",\n" +
                        "\"connected\": false,\n" +
                        "\"message\": \"NOT CONNECTED\"},\n" +
       "\"classes\": [{\"classid\": 1,\n" +
                    "\"count\": 17,\n" +
                    "\"nrounds\": 1,\n" +
                    "\"ntrophies\": -1,\n" +
                    "\"name\": \"Lions & Tigers\",\n" +
                    "\"subgroups\": [{\"rankid\": 1,\n" +
                                   "\"count\": 17,\n" +
                                   "\"name\": \"Lions & Tigers\"}]},\n" +
                   "{\"classid\": 2,\n" +
                    "\"count\": 18,\n" +
                    "\"nrounds\": 1,\n" +
                    "\"ntrophies\": -1,\n" +
                    "\"name\": \"White\'s Wolves\",\n" +
                    "\"subgroups\": [{\"rankid\": 2,\n" +
                                   "\"count\": 18,\n" +
                                   "\"name\": \"White\'s Wolves\"}]},\n" +
                   "{\"classid\": 3,\n" +
                    "\"count\": 17,\n" +
                    "\"nrounds\": 1,\n" +
                    "\"ntrophies\": -1,\n" +
                    "\"name\": \"Bears and Frèr\",\n" +
                    "\"subgroups\": [{\"rankid\": 3,\n" +
                                   "\"count\": 17,\n" +
                                   "\"name\": \"Bears and Frèr\"}]},\n" +
                   "{\"classid\": 4,\n" +
                    "\"count\": 16,\n" +
                    "\"nrounds\": 1,\n" +
                    "\"ntrophies\": -1,\n" +
                    "\"name\": \"Webelos (\\\"Webes\",\n" +
                    "\"subgroups\": [{\"rankid\": 4,\n" +
                                   "\"count\": 16,\n" +
                                   "\"name\": \"Webelos (\\\"Webes\"}]},\n" +
                   "{\"classid\": 5,\n" +
                    "\"count\": 15,\n" +
                    "\"nrounds\": 1,\n" +
                    "\"ntrophies\": -1,\n" +
                    "\"name\": \"Arrows <<--<<\",\n" +
                    "\"subgroups\": [{\"rankid\": 5,\n" +
                                   "\"count\": 15,\n" +
                                   "\"name\": \"Arrows <<--<<\"}]},\n" +
                   "{\"classid\": 7,\n" +
                    "\"count\": 0,\n" +
                    "\"nrounds\": 1,\n" +
                    "\"ntrophies\": -1,\n" +
                    "\"name\": \"TheLastClass\",\n" +
                    "\"subgroups\": [{\"rankid\": 7,\n" +
                                   "\"count\": 0,\n" +
                                   "\"name\": \"TheLastClass\"}]}],\n" +
       "\"rounds\": [{\"roundid\": 1,\n" +
                   "\"classid\": 1,\n" +
                   "\"class\": \"Lions & Tigers\",\n" +
                   "\"round\": 1,\n" +
                   "\"roster_size\": 17,\n" +
                   "\"passed\": 5,\n" +
                   "\"unscheduled\": 0,\n" +
                   "\"heats_scheduled\": 5,\n" +
                   "\"heats_run\": 5,\n" +
                   "\"name\": \"Lions & Tigers, Round 1\"},\n" +
                  "{\"roundid\": 2,\n" +
                   "\"classid\": 2,\n" +
                   "\"class\": \"White\'s Wolves\",\n" +
                   "\"round\": 1,\n" +
                   "\"roster_size\": 18,\n" +
                   "\"passed\": 5,\n" +
                   "\"unscheduled\": 0,\n" +
                   "\"heats_scheduled\": 5,\n" +
                   "\"heats_run\": 5,\n" +
                   "\"name\": \"White\'s Wolves, Round 1\"},\n" +
                  "{\"roundid\": 3,\n" +
                   "\"classid\": 3,\n" +
                   "\"class\": \"Bears and Frèr\",\n" +
                   "\"round\": 1,\n" +
                   "\"roster_size\": 17,\n" +
                   "\"passed\": 3,\n" +
                   "\"unscheduled\": 0,\n" +
                   "\"heats_scheduled\": 4,\n" +
                   "\"heats_run\": 2,\n" +
                   "\"name\": \"Bears and Frèr, Round 1\"},\n" +
                  "{\"roundid\": 4,\n" +
                   "\"classid\": 4,\n" +
                   "\"class\": \"Webelos (\\\"Webes\",\n" +
                   "\"round\": 1,\n" +
                   "\"roster_size\": 15,\n" +
                   "\"passed\": 2,\n" +
                   "\"unscheduled\": 2,\n" +
                   "\"heats_scheduled\": 0,\n" +
                   "\"heats_run\": 0,\n" +
                   "\"name\": \"Webelos (\\\"Webes, Round 1\"},\n" +
                  "{\"roundid\": 5,\n" +
                   "\"classid\": 5,\n" +
                   "\"class\": \"Arrows <<--<<\",\n" +
                   "\"round\": 1,\n" +
                   "\"roster_size\": 16,\n" +
                   "\"passed\": 5,\n" +
                   "\"unscheduled\": 5,\n" +
                   "\"heats_scheduled\": 0,\n" +
                   "\"heats_run\": 0,\n" +
                   "\"name\": \"Arrows <<--<<, Round 1\"},\n" +
                  "{\"roundid\": 7,\n" +
                   "\"classid\": 7,\n" +
                   "\"class\": \"TheLastClass\",\n" +
                   "\"round\": 1,\n" +
                   "\"roster_size\": 0,\n" +
                   "\"passed\": 0,\n" +
                   "\"unscheduled\": 0,\n" +
                   "\"heats_scheduled\": 0,\n" +
                   "\"heats_run\": 0,\n" +
                   "\"name\": \"TheLastClass, Round 1\"}]\n" +
                             "}");
  
  await page.waitForFunction(() => { return !$("#is-currently-racing").prop('checked'); });
  await page.waitForFunction(() => {
    return $("#now-racing-group .heat-text h3").text() == "Heat 4 of 4";
  });

  // After "Previous Heat" button jumps back to heat 3
  await fakeAjax.testForJson(async () => {
    var prev_button = await page.$("#prev_heat_button");
    prev_button.click();
  },
                       {'type': 'POST',
                        'data': {'action': 'heat.select',
                                 'heat': 'prev'}},
      "{\"current-heat\": {\"now_racing\": false,\n" +
                        "\"use_master_sched\": false,\n" +
                        "\"use_points\": false,\n" +
                             "\"classid\": 3,\n" +
                             "\"marker\": 497,\n" +
                        "\"roundid\": 3,\n" +
                        "\"round\": 1,\n" +
                        "\"tbodyid\": 3,\n" +
                        "\"heat\": 3,\n" +
                        "\"number-of-heats\": 4,\n" +
                      "\"class\": \"Bears and Frèr\"},\n" +
       "\"heat-results\": [],\n" +
       "\"ready-aggregate\": [],\n" +
       "\"racers\": [{\"lane\": 1,\n" +
                   "\"name\": \"Juan Jacobsen\",\n" +
                   "\"carname\": \"\",\n" +
                   "\"carnumber\": \"343\",\n" +
                   "\"photo\": \"\",\n" +
                   "\"finishtime\": \"\",\n" +
                   "\"finishplace\": \"\"},\n" +
                  "{\"lane\": 2,\n" +
                   "\"name\": \"Jeffress Jamison\",\n" +
                   "\"carname\": \"\",\n" +
                   "\"carnumber\": \"139\",\n" +
                   "\"photo\": \"\",\n" +
                   "\"finishtime\": \"\",\n" +
                   "\"finishplace\": \"\"},\n" +
                  "{\"lane\": 3,\n" +
                   "\"name\": \"Antoine Akiyama\",\n" +
                   "\"carname\": \"\",\n" +
                   "\"carnumber\": \"303\",\n" +
                   "\"photo\": \"\",\n" +
                   "\"finishtime\": \"\",\n" +
                   "\"finishplace\": \"\"}],\n" +
       "\"timer-state\": {\"lanes\": 4,\n" +
                       "\"last_contact\": 1526246081,\n" +
                       "\"state\": 3,\n" +
                       "\"icon\": \"img/status/ok.png\",\n" +
                       "\"message\": \"Staging\"},\n" +
       "\"replay-state\": {\"last_contact\": 0,\n" +
                        "\"state\": 1,\n" +
                        "\"icon\": \"img/status/not_connected.png\",\n" +
                        "\"connected\": false,\n" +
                        "\"message\": \"NOT CONNECTED\"},\n" +
       "\"classes\": [{\"classid\": 1,\n" +
                    "\"count\": 17,\n" +
                    "\"nrounds\": 1,\n" +
                    "\"ntrophies\": -1,\n" +
                    "\"name\": \"Lions & Tigers\",\n" +
                    "\"subgroups\": [{\"rankid\": 1,\n" +
                                   "\"count\": 17,\n" +
                                   "\"name\": \"Lions & Tigers\"}]},\n" +
                   "{\"classid\": 2,\n" +
                    "\"count\": 18,\n" +
                    "\"nrounds\": 1,\n" +
                    "\"ntrophies\": -1,\n" +
                    "\"name\": \"White\'s Wolves\",\n" +
                    "\"subgroups\": [{\"rankid\": 2,\n" +
                                   "\"count\": 18,\n" +
                                   "\"name\": \"White\'s Wolves\"}]},\n" +
                   "{\"classid\": 3,\n" +
                    "\"count\": 17,\n" +
                    "\"nrounds\": 1,\n" +
                    "\"ntrophies\": -1,\n" +
                    "\"name\": \"Bears and Frèr\",\n" +
                    "\"subgroups\": [{\"rankid\": 3,\n" +
                                   "\"count\": 17,\n" +
                                   "\"name\": \"Bears and Frèr\"}]},\n" +
                   "{\"classid\": 4,\n" +
                    "\"count\": 16,\n" +
                    "\"nrounds\": 1,\n" +
                    "\"ntrophies\": -1,\n" +
                    "\"name\": \"Webelos (\\\"Webes\",\n" +
                    "\"subgroups\": [{\"rankid\": 4,\n" +
                                   "\"count\": 16,\n" +
                                   "\"name\": \"Webelos (\\\"Webes\"}]},\n" +
                   "{\"classid\": 5,\n" +
                    "\"count\": 15,\n" +
                    "\"nrounds\": 1,\n" +
                    "\"ntrophies\": -1,\n" +
                    "\"name\": \"Arrows <<--<<\",\n" +
                    "\"subgroups\": [{\"rankid\": 1,\n" +
                                   "\"count\": 15,\n" +
                                   "\"name\": \"Arrows <<--<<\"}]},\n" +
                   "{\"classid\": 7,\n" +
                    "\"count\": 0,\n" +
                    "\"nrounds\": 1,\n" +
                    "\"ntrophies\": -1,\n" +
                    "\"name\": \"TheLastClass\",\n" +
                    "\"subgroups\": [{\"rankid\": 7,\n" +
                                   "\"count\": 0,\n" +
                                   "\"name\": \"TheLastClass\"}]}],\n" +
       "\"rounds\": [{\"roundid\": 1,\n" +
                   "\"classid\": 1,\n" +
                   "\"class\": \"Lions & Tigers\",\n" +
                   "\"round\": 1,\n" +
                   "\"roster_size\": 17,\n" +
                   "\"passed\": 5,\n" +
                   "\"unscheduled\": 0,\n" +
                   "\"heats_scheduled\": 5,\n" +
                   "\"heats_run\": 5,\n" +
                   "\"name\": \"Lions & Tigers, Round 1\"},\n" +
                  "{\"roundid\": 2,\n" +
                   "\"classid\": 2,\n" +
                   "\"class\": \"White\'s Wolves\",\n" +
                   "\"round\": 1,\n" +
                   "\"roster_size\": 18,\n" +
                   "\"passed\": 5,\n" +
                   "\"unscheduled\": 0,\n" +
                   "\"heats_scheduled\": 5,\n" +
                   "\"heats_run\": 5,\n" +
                   "\"name\": \"White\'s Wolves, Round 1\"},\n" +
                  "{\"roundid\": 3,\n" +
                   "\"classid\": 3,\n" +
                   "\"class\": \"Bears and Frèr\",\n" +
                   "\"round\": 1,\n" +
                   "\"roster_size\": 17,\n" +
                   "\"passed\": 3,\n" +
                   "\"unscheduled\": 0,\n" +
                   "\"heats_scheduled\": 4,\n" +
                   "\"heats_run\": 2,\n" +
                   "\"name\": \"Bears and Frèr, Round 1\"},\n" +
                  "{\"roundid\": 4,\n" +
                   "\"classid\": 4,\n" +
                   "\"class\": \"Webelos (\\\"Webes\",\n" +
                   "\"round\": 1,\n" +
                   "\"roster_size\": 15,\n" +
                   "\"passed\": 2,\n" +
                   "\"unscheduled\": 2,\n" +
                   "\"heats_scheduled\": 0,\n" +
                   "\"heats_run\": 0,\n" +
                   "\"name\": \"Webelos (\\\"Webes, Round 1\"},\n" +
                  "{\"roundid\": 5,\n" +
                   "\"classid\": 5,\n" +
                   "\"class\": \"Arrows <<--<<\",\n" +
                   "\"round\": 1,\n" +
                   "\"roster_size\": 16,\n" +
                   "\"passed\": 5,\n" +
                   "\"unscheduled\": 5,\n" +
                   "\"heats_scheduled\": 0,\n" +
                   "\"heats_run\": 0,\n" +
                   "\"name\": \"Arrows <<--<<, Round 1\"},\n" +
                  "{\"roundid\": 7,\n" +
                   "\"classid\": 7,\n" +
                   "\"class\": \"TheLastClass\",\n" +
                   "\"round\": 1,\n" +
                   "\"roster_size\": 0,\n" +
                   "\"passed\": 0,\n" +
                   "\"unscheduled\": 0,\n" +
                   "\"heats_scheduled\": 0,\n" +
                   "\"heats_run\": 0,\n" +
                   "\"name\": \"TheLastClass, Round 1\"}]\n" +
                             "}");

  await page.waitForFunction(() => { return !$("#is-currently-racing").prop('checked'); });
  await page.waitForFunction(() => {
    return $("#now-racing-group .heat-text h3").text() == "Heat 3 of 4";
  });

  if (false) {
  // Click "Now Racing" button to resume racing
  await fakeAjax.testForJson(async () => {
    var racing_button = await page.$(".ui-flipswitch > #is-currently-racing");
    racing_button.click();
  },
                       {'type': 'POST',
                        'data': {'action': 'heat.select',
                                 'now_racing': 1}},
      "{\"current-heat\": {\"now_racing\": true,\n" +
                        "\"use_master_sched\": false,\n" +
                        "\"use_points\": false,\n" +
                        "\"classid\": 3,\n" +
                        "\"roundid\": 3,\n" +
                        "\"round\": 1,\n" +
                        "\"tbodyid\": 3,\n" +
                        "\"heat\": 3,\n" +
                        "\"number-of-heats\": 4,\n" +
                      "\"class\": \"Bears and Frèr\"},\n" +
       "\"heat-results\": [],\n" +
       "\"ready-aggregate\": [],\n" +
       "\"racers\": [{\"lane\": 1,\n" +
                   "\"name\": \"Juan Jacobsen\",\n" +
                   "\"carname\": \"\",\n" +
                   "\"carnumber\": \"343\",\n" +
                   "\"photo\": \"\",\n" +
                   "\"finishtime\": \"\",\n" +
                   "\"finishplace\": \"\"},\n" +
                  "{\"lane\": 2,\n" +
                   "\"name\": \"Jeffress Jamison\",\n" +
                   "\"carname\": \"\",\n" +
                   "\"carnumber\": \"139\",\n" +
                   "\"photo\": \"\",\n" +
                   "\"finishtime\": \"\",\n" +
                   "\"finishplace\": \"\"},\n" +
                  "{\"lane\": 3,\n" +
                   "\"name\": \"Antoine Akiyama\",\n" +
                   "\"carname\": \"\",\n" +
                   "\"carnumber\": \"303\",\n" +
                   "\"photo\": \"\",\n" +
                   "\"finishtime\": \"\",\n" +
                   "\"finishplace\": \"\"}],\n" +
       "\"timer-state\": {\"lanes\": 4,\n" +
                       "\"last_contact\": 1526246081,\n" +
                       "\"state\": 3,\n" +
                       "\"icon\": \"img/status/ok.png\",\n" +
                       "\"message\": \"Staging\"},\n" +
       "\"replay-state\": {\"last_contact\": 0,\n" +
                        "\"state\": 1,\n" +
                        "\"icon\": \"img/status/not_connected.png\",\n" +
                        "\"connected\": false,\n" +
                        "\"message\": \"NOT CONNECTED\"},\n" +
       "\"classes\": [{\"classid\": 1,\n" +
                    "\"count\": 17,\n" +
                    "\"nrounds\": 1,\n" +
                    "\"ntrophies\": -1,\n" +
                    "\"name\": \"Lions & Tigers\",\n" +
                    "\"subgroups\": [{\"rankid\": 1,\n" +
                                   "\"count\": 17,\n" +
                                   "\"name\": \"Lions & Tigers\"}]},\n" +
                   "{\"classid\": 2,\n" +
                    "\"count\": 18,\n" +
                    "\"nrounds\": 1,\n" +
                    "\"ntrophies\": -1,\n" +
                    "\"name\": \"White\'s Wolves\",\n" +
                    "\"subgroups\": [{\"rankid\": 2,\n" +
                                   "\"count\": 18,\n" +
                                   "\"name\": \"White\'s Wolves\"}]},\n" +
                   "{\"classid\": 3,\n" +
                    "\"count\": 17,\n" +
                    "\"nrounds\": 1,\n" +
                    "\"ntrophies\": -1,\n" +
                    "\"name\": \"Bears and Frèr\",\n" +
                    "\"subgroups\": [{\"rankid\": 3,\n" +
                                   "\"count\": 17,\n" +
                                   "\"name\": \"Bears and Frèr\"}]},\n" +
                   "{\"classid\": 4,\n" +
                    "\"count\": 16,\n" +
                    "\"nrounds\": 1,\n" +
                    "\"ntrophies\": -1,\n" +
                    "\"name\": \"Webelos (\\\"Webes\",\n" +
                    "\"subgroups\": [{\"rankid\": 4,\n" +
                                   "\"count\": 16,\n" +
                                   "\"name\": \"Webelos (\\\"Webes\"}]},\n" +
                   "{\"classid\": 5,\n" +
                    "\"count\": 15,\n" +
                    "\"nrounds\": 1,\n" +
                    "\"ntrophies\": -1,\n" +
                    "\"name\": \"Arrows <<--<<\",\n" +
                    "\"subgroups\": [{\"rankid\": 1,\n" +
                                   "\"count\": 15,\n" +
                                   "\"name\": \"Arrows <<--<<\"}]},\n" +
                   "{\"classid\": 7,\n" +
                    "\"count\": 0,\n" +
                    "\"nrounds\": 1,\n" +
                    "\"ntrophies\": -1,\n" +
                    "\"name\": \"TheLastClass\",\n" +
                    "\"subgroups\": [{\"rankid\": 7,\n" +
                                   "\"count\": 0,\n" +
                                   "\"name\": \"TheLastClass\"}]}],\n" +
       "\"rounds\": [{\"roundid\": 1,\n" +
                   "\"classid\": 1,\n" +
                   "\"class\": \"Lions & Tigers\",\n" +
                   "\"round\": 1,\n" +
                   "\"roster_size\": 17,\n" +
                   "\"passed\": 5,\n" +
                   "\"unscheduled\": 0,\n" +
                   "\"heats_scheduled\": 5,\n" +
                   "\"heats_run\": 5,\n" +
                   "\"name\": \"Lions & Tigers, Round 1\"},\n" +
                  "{\"roundid\": 2,\n" +
                   "\"classid\": 2,\n" +
                   "\"class\": \"White\'s Wolves\",\n" +
                   "\"round\": 1,\n" +
                   "\"roster_size\": 18,\n" +
                   "\"passed\": 5,\n" +
                   "\"unscheduled\": 0,\n" +
                   "\"heats_scheduled\": 5,\n" +
                   "\"heats_run\": 5,\n" +
                   "\"name\": \"White\'s Wolves, Round 1\"},\n" +
                  "{\"roundid\": 3,\n" +
                   "\"classid\": 3,\n" +
                   "\"class\": \"Bears and Frèr\",\n" +
                   "\"round\": 1,\n" +
                   "\"roster_size\": 17,\n" +
                   "\"passed\": 3,\n" +
                   "\"unscheduled\": 0,\n" +
                   "\"heats_scheduled\": 4,\n" +
                   "\"heats_run\": 2,\n" +
                   "\"name\": \"Bears and Frèr, Round 1\"},\n" +
                  "{\"roundid\": 4,\n" +
                   "\"classid\": 4,\n" +
                   "\"class\": \"Webelos (\\\"Webes\",\n" +
                   "\"round\": 1,\n" +
                   "\"roster_size\": 15,\n" +
                   "\"passed\": 2,\n" +
                   "\"unscheduled\": 2,\n" +
                   "\"heats_scheduled\": 0,\n" +
                   "\"heats_run\": 0,\n" +
                   "\"name\": \"Webelos (\\\"Webes, Round 1\"},\n" +
                  "{\"roundid\": 5,\n" +
                   "\"classid\": 5,\n" +
                   "\"class\": \"Arrows <<--<<\",\n" +
                   "\"round\": 1,\n" +
                   "\"roster_size\": 16,\n" +
                   "\"passed\": 5,\n" +
                   "\"unscheduled\": 5,\n" +
                   "\"heats_scheduled\": 0,\n" +
                   "\"heats_run\": 0,\n" +
                   "\"name\": \"Arrows <<--<<, Round 1\"},\n" +
                  "{\"roundid\": 7,\n" +
                   "\"classid\": 7,\n" +
                   "\"class\": \"TheLastClass\",\n" +
                   "\"round\": 1,\n" +
                   "\"roster_size\": 0,\n" +
                   "\"passed\": 0,\n" +
                   "\"unscheduled\": 0,\n" +
                   "\"heats_scheduled\": 0,\n" +
                   "\"heats_run\": 0,\n" +
                   "\"name\": \"TheLastClass, Round 1\"}]\n" +
                             "}");

  await page.waitForFunction(() => { return $("#is-currently-racing").prop('checked'); });
  }

  // After manual results:
  var manual_results = await page.$("input[type='button'][value='Manual Results']");
  await fakeAjax.testForJson(async () => {
    manual_results.click();
    await page.waitForFunction(() => {
      var manual_results_modal = $("#manual_results_modal");
      return !manual_results_modal.hasClass('hidden') && manual_results_modal.css('opacity') >= 1;
    });

    // Cancel button dismisses #manual_results_modal
    await page.evaluate(() => { $("#manual_results_modal input[type='button'][value='Cancel']").click(); });
    await all_modals_closed();


    // Re-open the #manual_results_modal
    manual_results.click();
    await page.waitForFunction(() => {
      var manual_results_modal = $("#manual_results_modal");
      return !manual_results_modal.hasClass('hidden') && manual_results_modal.css('opacity') >= 1;
    });

    // Without this delay, there's some kind of race that results in an
    // {"action":"heat.rerun","heat":"current"} message in addition to the expected result.write message.
    await new Promise(r => setTimeout(r, 500));

    await page.evaluate(async () => {
      $("input[name='lane1']").val('1.234');
      $("input[name='lane2']").val('2.34');
      $("input[name='lane3']").val('4.321');
      $("#manual_results_modal input[type='submit']").click();
      return 0;
    });
  },
                       {'type': 'POST',
                        'data': 'action=result.write&lane1=1.234&lane2=2.34&lane3=4.321'},
      "{\"current-heat\": {\"now_racing\": true,\n" +
                        "\"use_master_sched\": false,\n" +
                        "\"use_points\": false,\n" +
                             "\"classid\": 3,\n" +
                             "\"marker\": 856,\n" +
                        "\"roundid\": 3,\n" +
                        "\"round\": 1,\n" +
                        "\"tbodyid\": 3,\n" +
                        "\"heat\": 3,\n" +
                        "\"number-of-heats\": 4,\n" +
                      "\"class\": \"Bears and Frèr\"},\n" +
       "\"heat-results\": [],\n" +
       "\"ready-aggregate\": [],\n" +
       "\"racers\": [{\"lane\": 1,\n" +
                   "\"name\": \"Juan Jacobsen\",\n" +
                   "\"carname\": \"\",\n" +
                   "\"carnumber\": \"343\",\n" +
                   "\"photo\": \"\",\n" +
                   "\"finishtime\": \"1.234\",\n" +
                   "\"finishplace\": 1},\n" +
                  "{\"lane\": 2,\n" +
                   "\"name\": \"Jeffress Jamison\",\n" +
                   "\"carname\": \"\",\n" +
                   "\"carnumber\": \"139\",\n" +
                   "\"photo\": \"\",\n" +
                   "\"finishtime\": \"2.34\",\n" +
                   "\"finishplace\": 2},\n" +
                  "{\"lane\": 3,\n" +
                   "\"name\": \"Antoine Akiyama\",\n" +
                   "\"carname\": \"\",\n" +
                   "\"carnumber\": \"303\",\n" +
                   "\"photo\": \"\",\n" +
                   "\"finishtime\": \"4.321\",\n" +
                   "\"finishplace\": 3}],\n" +
       "\"timer-state\": {\"lanes\": 4,\n" +
                       "\"last_contact\": 1526246081,\n" +
                       "\"state\": 3,\n" +
                       "\"icon\": \"img/status/ok.png\",\n" +
                       "\"message\": \"Staging\"},\n" +
       "\"replay-state\": {\"last_contact\": 0,\n" +
                        "\"state\": 1,\n" +
                        "\"icon\": \"img/status/not_connected.png\",\n" +
                        "\"connected\": false,\n" +
                        "\"message\": \"NOT CONNECTED\"},\n" +
       "\"classes\": [{\"classid\": 1,\n" +
                    "\"count\": 17,\n" +
                    "\"nrounds\": 1,\n" +
                    "\"ntrophies\": -1,\n" +
                    "\"name\": \"Lions &amp; Tigers\",\n" +
                    "\"subgroups\": [{\"rankid\": 1,\n" +
                                   "\"count\": 17,\n" +
                                   "\"name\": \"Lions &amp; Tigers\"}]},\n" +
                   "{\"classid\": 2,\n" +
                    "\"count\": 18,\n" +
                    "\"nrounds\": 1,\n" +
                    "\"ntrophies\": -1,\n" +
                    "\"name\": \"White\'s Wolves\",\n" +
                    "\"subgroups\": [{\"rankid\": 2,\n" +
                                   "\"count\": 18,\n" +
                                   "\"name\": \"White\'s Wolves\"}]},\n" +
                   "{\"classid\": 3,\n" +
                    "\"count\": 17,\n" +
                    "\"nrounds\": 1,\n" +
                    "\"ntrophies\": -1,\n" +
                    "\"name\": \"Bears and Frèr\",\n" +
                    "\"subgroups\": [{\"rankid\": 3,\n" +
                                   "\"count\": 17,\n" +
                                   "\"name\": \"Bears and Frèr\"}]},\n" +
                   "{\"classid\": 4,\n" +
                    "\"count\": 16,\n" +
                    "\"nrounds\": 1,\n" +
                    "\"ntrophies\": -1,\n" +
                    "\"name\": \"Webelos (\\\"Webes\",\n" +
                    "\"subgroups\": [{\"rankid\": 4,\n" +
                                   "\"count\": 16,\n" +
                                   "\"name\": \"Webelos (\\\"Webes\"}]},\n" +
                   "{\"classid\": 5,\n" +
                    "\"count\": 15,\n" +
                    "\"nrounds\": 1,\n" +
                    "\"ntrophies\": -1,\n" +
                    "\"name\": \"Arrows &lt;&lt;--&lt;&lt;\",\n" +
                    "\"subgroups\": [{\"rankid\": 5,\n" +
                                   "\"count\": 15,\n" +
                                   "\"name\": \"Arrows &lt;&lt;--&lt;&lt;\"}]},\n" +
                   "{\"classid\": 7,\n" +
                    "\"count\": 0,\n" +
                    "\"nrounds\": 1,\n" +
                    "\"ntrophies\": -1,\n" +
                    "\"name\": \"TheLastClass\",\n" +
                    "\"subgroups\": [{\"rankid\": 7,\n" +
                                   "\"count\": 0,\n" +
                                   "\"name\": \"TheLastClass\"}]}],\n" +
       "\"rounds\": [{\"roundid\": 1,\n" +
                   "\"classid\": 1,\n" +
                   "\"class\": \"Lions &amp; Tigers\",\n" +
                   "\"round\": 1,\n" +
                   "\"roster_size\": 17,\n" +
                   "\"passed\": 5,\n" +
                   "\"unscheduled\": 0,\n" +
                   "\"heats_scheduled\": 5,\n" +
                   "\"heats_run\": 5,\n" +
                   "\"name\": \"Lions &amp; Tigers, Round 1\"},\n" +
                  "{\"roundid\": 2,\n" +
                   "\"classid\": 2,\n" +
                   "\"class\": \"White\'s Wolves\",\n" +
                   "\"round\": 1,\n" +
                   "\"roster_size\": 18,\n" +
                   "\"passed\": 5,\n" +
                   "\"unscheduled\": 0,\n" +
                   "\"heats_scheduled\": 5,\n" +
                   "\"heats_run\": 5,\n" +
                   "\"name\": \"White\'s Wolves, Round 1\"},\n" +
                  "{\"roundid\": 3,\n" +
                   "\"classid\": 3,\n" +
                   "\"class\": \"Bears and Frèr\",\n" +
                   "\"round\": 1,\n" +
                   "\"roster_size\": 15,\n" +
                   "\"passed\": 3,\n" +
                   "\"unscheduled\": 0,\n" +
                   "\"heats_scheduled\": 4,\n" +
                   "\"heats_run\": 2,\n" +
                   "\"name\": \"Bears and Frèr, Round 1\"},\n" +
                  "{\"roundid\": 4,\n" +
                   "\"classid\": 4,\n" +
                   "\"class\": \"Webelos (\\\"Webes\",\n" +
                   "\"round\": 1,\n" +
                   "\"roster_size\": 15,\n" +
                   "\"passed\": 2,\n" +
                   "\"unscheduled\": 2,\n" +
                   "\"heats_scheduled\": 0,\n" +
                   "\"heats_run\": 0,\n" +
                   "\"name\": \"Webelos (\\\"Webes, Round 1\"},\n" +
                  "{\"roundid\": 5,\n" +
                   "\"classid\": 5,\n" +
                   "\"class\": \"Arrows &lt;&lt;--&lt;&lt;\",\n" +
                   "\"round\": 1,\n" +
                   "\"roster_size\": 16,\n" +
                   "\"passed\": 5,\n" +
                   "\"unscheduled\": 5,\n" +
                   "\"heats_scheduled\": 0,\n" +
                   "\"heats_run\": 0,\n" +
                   "\"name\": \"Arrows &lt;&lt;--&lt;&lt;, Round 1\"},\n" +
                  "{\"roundid\": 7,\n" +
                   "\"classid\": 7,\n" +
                   "\"class\": \"TheLastClass\",\n" +
                   "\"round\": 1,\n" +
                   "\"roster_size\": 0,\n" +
                   "\"passed\": 0,\n" +
                   "\"unscheduled\": 0,\n" +
                   "\"heats_scheduled\": 0,\n" +
                   "\"heats_run\": 0,\n" +
                   "\"name\": \"TheLastClass, Round 1\"}]\n" +
                             "}");

  // Check manual results dialog dismissed
  await all_modals_closed();

  await page.waitForFunction(() => {
    var row1 = $("#now-racing-group table tr")[1];
    var cell1_3 = $(row1).find("td")[3];
    var row2 = $("#now-racing-group table tr")[2];
    var cell2_3 = $(row2).find("td")[3];
    return $(cell1_3).text() == "1.234" && $(cell2_3).text() == "2.34";
  });

  // Re-open the #manual_results_modal
  manual_results.click();
  await page.waitForFunction(() => {
    var manual_results_modal = $("#manual_results_modal");
    return !manual_results_modal.hasClass('hidden') && manual_results_modal.css('opacity') >= 1;
  });

  await fakeAjax.testForJson(async () => {
    await page.evaluate(() => { $("#discard-results").click(); });
    await all_modals_closed();
  },
                       {'type': 'POST',
                        'data': {"action": "result.delete",
                                 "roundid": "current",
                                 "heat": "current"}},
      "{\"current-heat\": {\"now_racing\": true,\n" +
                        "\"use_master_sched\": false,\n" +
                        "\"use_points\": false,\n" +
                             "\"classid\": 3,\n" +
                             "\"marker\": 1038,\n" +
                        "\"roundid\": 3,\n" +
                        "\"round\": 1,\n" +
                        "\"tbodyid\": 3,\n" +
                        "\"heat\": 3,\n" +
                        "\"number-of-heats\": 4,\n" +
                      "\"class\": \"Bears and Frèr\"},\n" +
       "\"heat-results\": [],\n" +
       "\"ready-aggregate\": [],\n" +
       "\"racers\": [{\"lane\": 1,\n" +
                   "\"name\": \"Juan Jacobsen\",\n" +
                   "\"carname\": \"\",\n" +
                   "\"carnumber\": \"343\",\n" +
                   "\"photo\": \"\",\n" +
                   "\"finishtime\": \"\",\n" +
                   "\"finishplace\": \"\"},\n" +
                  "{\"lane\": 2,\n" +
                   "\"name\": \"Jeffress Jamison\",\n" +
                   "\"carname\": \"\",\n" +
                   "\"carnumber\": \"139\",\n" +
                   "\"photo\": \"\",\n" +
                   "\"finishtime\": \"\",\n" +
                   "\"finishplace\": \"\"},\n" +
                  "{\"lane\": 3,\n" +
                   "\"name\": \"Antoine Akiyama\",\n" +
                   "\"carname\": \"\",\n" +
                   "\"carnumber\": \"303\",\n" +
                   "\"photo\": \"\",\n" +
                   "\"finishtime\": \"\",\n" +
                   "\"finishplace\": \"\"}],\n" +
       "\"timer-state\": {\"lanes\": 4,\n" +
                       "\"last_contact\": 1526246081,\n" +
                       "\"state\": 3,\n" +
                       "\"icon\": \"img/status/ok.png\",\n" +
                       "\"message\": \"Staging\"},\n" +
       "\"replay-state\": {\"last_contact\": 0,\n" +
                        "\"state\": 1,\n" +
                        "\"icon\": \"img/status/not_connected.png\",\n" +
                        "\"connected\": false,\n" +
                        "\"message\": \"NOT CONNECTED\"},\n" +
       "\"classes\": [{\"classid\": 1,\n" +
                    "\"count\": 17,\n" +
                    "\"nrounds\": 1,\n" +
                    "\"ntrophies\": -1,\n" +
                    "\"name\": \"Lions &amp; Tigers\",\n" +
                    "\"subgroups\": [{\"rankid\": 1,\n" +
                                   "\"count\": 17,\n" +
                                   "\"name\": \"Lions &amp; Tigers\"}]},\n" +
                   "{\"classid\": 2,\n" +
                    "\"count\": 18,\n" +
                    "\"nrounds\": 1,\n" +
                    "\"ntrophies\": -1,\n" +
                    "\"name\": \"White\'s Wolves\",\n" +
                    "\"subgroups\": [{\"rankid\": 2,\n" +
                                   "\"count\": 18,\n" +
                                   "\"name\": \"White\'s Wolves\"}]},\n" +
                   "{\"classid\": 3,\n" +
                    "\"count\": 17,\n" +
                    "\"nrounds\": 1,\n" +
                    "\"ntrophies\": -1,\n" +
                    "\"name\": \"Bears and Frèr\",\n" +
                    "\"subgroups\": [{\"rankid\": 3,\n" +
                                   "\"count\": 17,\n" +
                                   "\"name\": \"Bears and Frèr\"}]},\n" +
                   "{\"classid\": 4,\n" +
                    "\"count\": 16,\n" +
                    "\"nrounds\": 1,\n" +
                    "\"ntrophies\": -1,\n" +
                    "\"name\": \"Webelos (\\\"Webes\",\n" +
                    "\"subgroups\": [{\"rankid\": 4,\n" +
                                   "\"count\": 16,\n" +
                                   "\"name\": \"Webelos (\\\"Webes\"}]},\n" +
                   "{\"classid\": 5,\n" +
                    "\"count\": 15,\n" +
                    "\"nrounds\": 1,\n" +
                    "\"ntrophies\": -1,\n" +
                    "\"name\": \"Arrows &lt;&lt;--&lt;&lt;\",\n" +
                    "\"subgroups\": [{\"rankid\": 5,\n" +
                                   "\"count\": 15,\n" +
                                   "\"name\": \"Arrows &lt;&lt;--&lt;&lt;\"}]},\n" +
                   "{\"classid\": 7,\n" +
                    "\"count\": 0,\n" +
                    "\"nrounds\": 1,\n" +
                    "\"ntrophies\": -1,\n" +
                    "\"name\": \"TheLastClass\",\n" +
                    "\"subgroups\": [{\"rankid\": 7,\n" +
                                   "\"count\": 0,\n" +
                                   "\"name\": \"TheLastClass\"}]}],\n" +
       "\"rounds\": [{\"roundid\": 1,\n" +
                   "\"classid\": 1,\n" +
                   "\"class\": \"Lions &amp; Tigers\",\n" +
                   "\"round\": 1,\n" +
                   "\"roster_size\": 17,\n" +
                   "\"passed\": 5,\n" +
                   "\"unscheduled\": 0,\n" +
                   "\"heats_scheduled\": 5,\n" +
                   "\"heats_run\": 5,\n" +
                   "\"name\": \"Lions &amp; Tigers, Round 1\"},\n" +
                  "{\"roundid\": 2,\n" +
                   "\"classid\": 2,\n" +
                   "\"class\": \"White\'s Wolves\",\n" +
                   "\"round\": 1,\n" +
                   "\"roster_size\": 18,\n" +
                   "\"passed\": 5,\n" +
                   "\"unscheduled\": 0,\n" +
                   "\"heats_scheduled\": 5,\n" +
                   "\"heats_run\": 5,\n" +
                   "\"name\": \"White\'s Wolves, Round 1\"},\n" +
                  "{\"roundid\": 3,\n" +
                   "\"classid\": 3,\n" +
                   "\"class\": \"Bears and Frèr\",\n" +
                   "\"round\": 1,\n" +
                   "\"roster_size\": 15,\n" +
                   "\"passed\": 3,\n" +
                   "\"unscheduled\": 0,\n" +
                   "\"heats_scheduled\": 4,\n" +
                   "\"heats_run\": 2,\n" +
                   "\"name\": \"Bears and Frèr, Round 1\"},\n" +
                  "{\"roundid\": 4,\n" +
                   "\"classid\": 4,\n" +
                   "\"class\": \"Webelos (\\\"Webes\",\n" +
                   "\"round\": 1,\n" +
                   "\"roster_size\": 15,\n" +
                   "\"passed\": 2,\n" +
                   "\"unscheduled\": 2,\n" +
                   "\"heats_scheduled\": 0,\n" +
                   "\"heats_run\": 0,\n" +
                   "\"name\": \"Webelos (\\\"Webes, Round 1\"},\n" +
                  "{\"roundid\": 5,\n" +
                   "\"classid\": 5,\n" +
                   "\"class\": \"Arrows &lt;&lt;--&lt;&lt;\",\n" +
                   "\"round\": 1,\n" +
                   "\"roster_size\": 16,\n" +
                   "\"passed\": 5,\n" +
                   "\"unscheduled\": 5,\n" +
                   "\"heats_scheduled\": 0,\n" +
                   "\"heats_run\": 0,\n" +
                   "\"name\": \"Arrows &lt;&lt;--&lt;&lt;, Round 1\"},\n" +
                  "{\"roundid\": 7,\n" +
                   "\"classid\": 7,\n" +
                   "\"class\": \"TheLastClass\",\n" +
                   "\"round\": 1,\n" +
                   "\"roster_size\": 0,\n" +
                   "\"passed\": 0,\n" +
                   "\"unscheduled\": 0,\n" +
                   "\"heats_scheduled\": 0,\n" +
                   "\"heats_run\": 0,\n" +
                   "\"name\": \"TheLastClass, Round 1\"}]\n" +
                             "}");

  await page.waitForFunction(() => {
    var row1 = $("#now-racing-group table tr")[1];
    var cell1_3 = $(row1).find("td")[3];
    var row2 = $("#now-racing-group table tr")[2];
    var cell2_3 = $(row2).find("td")[3];
    return $(cell1_3).text() == "" && $(cell2_3).text() == "";
  });

  // Click "Add New Rounds" button, see the dialog, dismiss it.
  await page.evaluate(() => { $("input[type='button'][value='Add New Rounds']").click(); });
  await modal_open("#choose_new_round_modal");
  await page.evaluate(() => { $("#choose_new_round_modal input[type='button'][value='Cancel'").click(); });
  await all_modals_closed();

  // Click "Add New Rounds" button, see the dialog, choose a den, see #new-round-modal, dismiss it.
  await page.evaluate(() => { $("input[type='button'][value='Add New Rounds']").click(); });
  await modal_open("#choose_new_round_modal");
  await page.evaluate(() => { $($("#choose_new_round_modal input[type='button']")[1]).click(); });
  await modal_open("#new-round-modal");
  await page.evaluate(() => { $("#new-round-modal input[type='button'][value='Cancel']").click(); });
  await all_modals_closed();

  await fakeAjax.testForJson(async () => {
    await page.evaluate(() => { $("input[type='button'][value='Add New Rounds']").click(); });
    await modal_open("#choose_new_round_modal");

    // Buttons in the "Add New Rounds" modal:
    //  Lions & Tigers
    //  White's Wolves [1]
    //  Aggregate Round
    //  Cancel
    await page.evaluate(() => { $($("#choose_new_round_modal input[type='button']")[1]).click(); });
    await modal_open("#new-round-modal");
    await page.evaluate(() => { $("#new-round-modal input[type='number'][name='top']").val('4'); });
    await page.waitForFunction(() => { return $(".aggregate-only").hasClass('hidden'); });
    await page.evaluate(() => { $("#new-round-modal input[type='submit']").click(); });
  },
                             {'type': 'POST',
                              'data': {"action":"roster.new","roundid":2,"top":"4","bucketed":0}
                             },
      "{\"current-heat\": {\"now_racing\": false,\n" +
                        "\"use_master_sched\": false,\n" +
                        "\"use_points\": false,\n" +
                             "\"classid\": 2,\n" +
                        "\"roundid\": 2,\n" +
                        "\"round\": 1,\n" +
                        "\"tbodyid\": 2,\n" +
                        "\"heat\": 5,\n" +
                        "\"number-of-heats\": 5,\n" +
                      "\"class\": \"White\'s Wolves\"},\n" +
       "\"heat-results\": [],\n" +
       "\"ready-aggregate\": [],\n" +
       "\"racers\": [{\"lane\": 1,\n" +
                   "\"name\": \"Kelvin Knapp\",\n" +
                   "\"carname\": \"\",\n" +
                   "\"carnumber\": \"247\",\n" +
                   "\"photo\": \"\",\n" +
                   "\"finishtime\": \"\",\n" +
                   "\"finishplace\": \"\"},\n" +
                  "{\"lane\": 3,\n" +
                   "\"name\": \"Darrell &amp; Darrell Delaughter\",\n" +
                   "\"carname\": \"\",\n" +
                   "\"carnumber\": \"217\",\n" +
                   "\"photo\": \"\",\n" +
                   "\"finishtime\": \"\",\n" +
                   "\"finishplace\": \"\"},\n" +
                  "{\"lane\": 5,\n" +
                   "\"name\": \"Ian Ives\",\n" +
                   "\"carname\": \"\",\n" +
                   "\"carnumber\": \"237\",\n" +
                   "\"photo\": \"\",\n" +
                   "\"finishtime\": \"\",\n" +
                   "\"finishplace\": \"\"}],\n" +
       "\"timer-state\": {\"lanes\": 4,\n" +
                       "\"last_contact\": 1526246081,\n" +
                       "\"state\": 3,\n" +
                       "\"icon\": \"img/status/ok.png\",\n" +
                       "\"message\": \"Staging\"},\n" +
       "\"replay-state\": {\"last_contact\": 0,\n" +
                        "\"state\": 1,\n" +
                        "\"icon\": \"img/status/not_connected.png\",\n" +
                        "\"connected\": false,\n" +
                        "\"message\": \"NOT CONNECTED\"},\n" +
       "\"classes\": [{\"classid\": 1,\n" +
                    "\"count\": 17,\n" +
                    "\"nrounds\": 1,\n" +
                    "\"ntrophies\": -1,\n" +
                    "\"name\": \"Lions &amp; Tigers\",\n" +
                    "\"subgroups\": [{\"rankid\": 1,\n" +
                                   "\"count\": 17,\n" +
                                   "\"name\": \"Lions &amp; Tigers\"}]},\n" +
                   "{\"classid\": 2,\n" +
                    "\"count\": 18,\n" +
                    "\"nrounds\": 1,\n" +
                    "\"ntrophies\": -1,\n" +
                    "\"name\": \"White\'s Wolves\",\n" +
                    "\"subgroups\": [{\"rankid\": 2,\n" +
                                   "\"count\": 18,\n" +
                                   "\"name\": \"White\'s Wolves\"}]},\n" +
                   "{\"classid\": 3,\n" +
                    "\"count\": 17,\n" +
                    "\"nrounds\": 1,\n" +
                    "\"ntrophies\": -1,\n" +
                    "\"name\": \"Bears and Frèr\",\n" +
                    "\"subgroups\": [{\"rankid\": 3,\n" +
                                   "\"count\": 17,\n" +
                                   "\"name\": \"Bears and Frèr\"}]},\n" +
                   "{\"classid\": 4,\n" +
                    "\"count\": 16,\n" +
                    "\"nrounds\": 1,\n" +
                    "\"ntrophies\": -1,\n" +
                    "\"name\": \"Webelos (\\\"Webes\",\n" +
                    "\"subgroups\": [{\"rankid\": 4,\n" +
                                   "\"count\": 16,\n" +
                                   "\"name\": \"Webelos (\\\"Webes\"}]},\n" +
                   "{\"classid\": 5,\n" +
                    "\"count\": 15,\n" +
                    "\"nrounds\": 1,\n" +
                    "\"ntrophies\": -1,\n" +
                    "\"name\": \"Arrows &lt;&lt;--&lt;&lt;\",\n" +
                    "\"subgroups\": [{\"rankid\": 5,\n" +
                                   "\"count\": 15,\n" +
                                   "\"name\": \"Arrows &lt;&lt;--&lt;&lt;\"}]},\n" +
                   "{\"classid\": 7,\n" +
                    "\"count\": 0,\n" +
                    "\"nrounds\": 1,\n" +
                    "\"ntrophies\": -1,\n" +
                    "\"name\": \"TheLastClass\",\n" +
                    "\"subgroups\": [{\"rankid\": 7,\n" +
                                   "\"count\": 0,\n" +
                                   "\"name\": \"TheLastClass\"}]}],\n" +
       "\"rounds\": [{\"roundid\": 1,\n" +
                   "\"classid\": 1,\n" +
                   "\"class\": \"Lions &amp; Tigers\",\n" +
                   "\"round\": 1,\n" +
                   "\"roster_size\": 17,\n" +
                   "\"passed\": 5,\n" +
                   "\"unscheduled\": 0,\n" +
                   "\"heats_scheduled\": 5,\n" +
                   "\"heats_run\": 5,\n" +
                   "\"name\": \"Lions &amp; Tigers, Round 1\"},\n" +
                  "{\"roundid\": 2,\n" +
                   "\"classid\": 2,\n" +
                   "\"class\": \"White\'s Wolves\",\n" +
                   "\"round\": 1,\n" +
                   "\"roster_size\": 18,\n" +
                   "\"passed\": 5,\n" +
                   "\"unscheduled\": 0,\n" +
                   "\"heats_scheduled\": 5,\n" +
                   "\"heats_run\": 5,\n" +
                   "\"name\": \"White\'s Wolves, Round 1\"},\n" +
                  "{\"roundid\": 3,\n" +
                   "\"classid\": 3,\n" +
                   "\"class\": \"Bears and Frèr\",\n" +
                   "\"round\": 1,\n" +
                   "\"roster_size\": 15,\n" +
                   "\"passed\": 3,\n" +
                   "\"unscheduled\": 0,\n" +
                   "\"heats_scheduled\": 4,\n" +
                   "\"heats_run\": 2,\n" +
                   "\"name\": \"Bears and Frèr, Round 1\"},\n" +
                  "{\"roundid\": 4,\n" +
                   "\"classid\": 4,\n" +
                   "\"class\": \"Webelos (\\\"Webes\",\n" +
                   "\"round\": 1,\n" +
                   "\"roster_size\": 15,\n" +
                   "\"passed\": 2,\n" +
                   "\"unscheduled\": 2,\n" +
                   "\"heats_scheduled\": 0,\n" +
                   "\"heats_run\": 0,\n" +
                   "\"name\": \"Webelos (\\\"Webes, Round 1\"},\n" +
                  "{\"roundid\": 5,\n" +
                   "\"classid\": 5,\n" +
                   "\"class\": \"Arrows &lt;&lt;--&lt;&lt;\",\n" +
                   "\"round\": 1,\n" +
                   "\"roster_size\": 16,\n" +
                   "\"passed\": 5,\n" +
                   "\"unscheduled\": 5,\n" +
                   "\"heats_scheduled\": 0,\n" +
                   "\"heats_run\": 0,\n" +
                   "\"name\": \"Arrows &lt;&lt;--&lt;&lt;, Round 1\"},\n" +
                  "{\"roundid\": 7,\n" +
                   "\"classid\": 7,\n" +
                   "\"class\": \"TheLastClass\",\n" +
                   "\"round\": 1,\n" +
                   "\"roster_size\": 0,\n" +
                   "\"passed\": 0,\n" +
                   "\"unscheduled\": 0,\n" +
                   "\"heats_scheduled\": 0,\n" +
                   "\"heats_run\": 0,\n" +
                   "\"name\": \"TheLastClass, Round 1\"}]\n" +
                             "}");

  // TODO Try creating a new aggregate round

  // TODO input[type='button'][value='Replay Settings']
  
  // =============================== Schedule a new round by clicking buttons =====================================
  await fakeAjax.testForJson(async () => {
    // Click control group to expose schedule button
    await open_scheduling_control_group(".control_group[data-roundid='5']");
    // Click Schedule button to open schedule modal
    await page.evaluate(() => {
      $(".control_group[data-roundid='5'] input[type='button'][value='Schedule']").click();
    });
    // Wait for the modal to appear
    await page.waitForFunction(() => {
      var schedule_modal = $("#schedule_modal");
      return !schedule_modal.hasClass('hidden') && schedule_modal.css('opacity') >= 1;
    });
    // Click the Schedule + Race button
    var dialog_schedule_and_race =
        await page.waitForSelector("#schedule_modal input[type='submit'][data-race='true']",
                                   {visible: true});
    await dialog_schedule_and_race.click();
  },
                       {'type': 'POST',
                        'data': {'action': 'schedule.generate',
                                 'roundid': 5,
                                 'n_times_per_lane': '1' } },
                         false);  // No xml response

  // ================================= Simulated poll =============================================================

  var poll_result =
      "{\"current-heat\": {\"now_racing\": true,\n" +
                        "\"use_master_sched\": true,\n" +
                        "\"use_points\": false,\n" +
                             "\"classid\": 2,\n" +
                        "\"roundid\": 2,\n" +
                        "\"round\": 1,\n" +
                        "\"tbodyid\": 2,\n" +
                        "\"heat\": 13,\n" +
                        "\"number-of-heats\": 13,\n" +
                      "\"class\": \"White\'s Wolves\"},\n" +
       "\"heat-results\": [],\n" +
       "\"ready-aggregate\": [],\n" +
       "\"racers\": [{\"lane\": 1,\n" +
                   "\"name\": \"Willard Woolfolk\",\n" +
                   "\"carname\": \"\",\n" +
                   "\"carnumber\": \"282\",\n" +
                   "\"photo\": \"\",\n" +
                   "\"finishtime\": \"3.977\",\n" +
                   "\"finishplace\": 4},\n" +
                  "{\"lane\": 2,\n" +
                   "\"name\": \"Blake Burling\",\n" +
                   "\"carname\": \"\",\n" +
                   "\"carnumber\": \"207\",\n" +
                   "\"photo\": \"\",\n" +
                   "\"finishtime\": \"3.646\",\n" +
                   "\"finishplace\": 2},\n" +
                  "{\"lane\": 3,\n" +
                   "\"name\": \"Elliot Eastman\",\n" +
                   "\"carname\": \"\",\n" +
                   "\"carnumber\": \"227\",\n" +
                   "\"photo\": \"\",\n" +
                   "\"finishtime\": \"2.295\",\n" +
                   "\"finishplace\": 1},\n" +
                  "{\"lane\": 4,\n" +
                   "\"name\": \"Dexter Dawes\",\n" +
                   "\"carname\": \"\",\n" +
                   "\"carnumber\": \"222\",\n" +
                   "\"photo\": \"\",\n" +
                   "\"finishtime\": \"3.720\",\n" +
                   "\"finishplace\": 3}],\n" +
       "\"timer-state\": {\"lanes\": 4,\n" +
                       "\"last_contact\": 1526246081,\n" +
                       "\"state\": 3,\n" +
                       "\"icon\": \"img/status/ok.png\",\n" +
                       "\"message\": \"Staging\"},\n" +
       "\"replay-state\": {\"last_contact\": 0,\n" +
                        "\"state\": 1,\n" +
                        "\"icon\": \"img/status/not_connected.png\",\n" +
                        "\"connected\": false,\n" +
                        "\"message\": \"NOT CONNECTED\"},\n" +
       "\"classes\": [{\"classid\": 1,\n" +
                    "\"count\": 17,\n" +
                    "\"nrounds\": 1,\n" +
                    "\"ntrophies\": -1,\n" +
                    "\"name\": \"Lions &amp; Tigers\",\n" +
                    "\"subgroups\": [{\"rankid\": 1,\n" +
                                   "\"count\": 17,\n" +
                                   "\"name\": \"Lions &amp; Tigers\"}]},\n" +
                   "{\"classid\": 2,\n" +
                    "\"count\": 18,\n" +
                    "\"nrounds\": 1,\n" +
                    "\"ntrophies\": -1,\n" +
                    "\"name\": \"White\'s Wolves\",\n" +
                    "\"subgroups\": [{\"rankid\": 2,\n" +
                                   "\"count\": 18,\n" +
                                   "\"name\": \"White\'s Wolves\"}]},\n" +
                   "{\"classid\": 3,\n" +
                    "\"count\": 17,\n" +
                    "\"nrounds\": 1,\n" +
                    "\"ntrophies\": -1,\n" +
                    "\"name\": \"Bears and Frèr\",\n" +
                    "\"subgroups\": [{\"rankid\": 3,\n" +
                                   "\"count\": 17,\n" +
                                   "\"name\": \"Bears and Frèr\"}]},\n" +
                   "{\"classid\": 4,\n" +
                    "\"count\": 16,\n" +
                    "\"nrounds\": 1,\n" +
                    "\"ntrophies\": -1,\n" +
                    "\"name\": \"Webelos (\\\"Webes\",\n" +
                    "\"subgroups\": [{\"rankid\": 4,\n" +
                                   "\"count\": 16,\n" +
                                   "\"name\": \"Webelos (\\\"Webes\"}]},\n" +
                   "{\"classid\": 5,\n" +
                    "\"count\": 15,\n" +
                    "\"nrounds\": 1,\n" +
                    "\"ntrophies\": -1,\n" +
                    "\"name\": \"Arrows &lt;&lt;--&lt;&lt;\",\n" +
                    "\"subgroups\": [{\"rankid\": 5,\n" +
                                   "\"count\": 15,\n" +
                                   "\"name\": \"Arrows &lt;&lt;--&lt;&lt;\"}]},\n" +
                   "{\"classid\": 7,\n" +
                    "\"count\": 0,\n" +
                    "\"nrounds\": 1,\n" +
                    "\"ntrophies\": -1,\n" +
                    "\"name\": \"TheLastClass\",\n" +
                    "\"subgroups\": [{\"rankid\": 7,\n" +
                                   "\"count\": 0,\n" +
                                   "\"name\": \"TheLastClass\"}]}],\n" +
       "\"rounds\": [{\"roundid\": 7,\n" +
                   "\"classid\": 2,\n" +
                   "\"class\": \"White\'s Wolves\",\n" +
                   "\"round\": 2,\n" +
                   "\"roster_size\": 3,\n" +
                   "\"passed\": 3,\n" +
                   "\"unscheduled\": 3,\n" +
                   "\"heats_scheduled\": 0,\n" +
                   "\"heats_run\": 0,\n" +
                   "\"name\": \"White\'s Wolves, Round 2\"},\n" +
                  "{\"roundid\": 1,\n" +
                   "\"classid\": 1,\n" +
                   "\"class\": \"Lions &amp; Tigers\",\n" +
                   "\"round\": 1,\n" +
                   "\"roster_size\": 17,\n" +
                   "\"passed\": 17,\n" +
                   "\"unscheduled\": 0,\n" +
                   "\"heats_scheduled\": 17,\n" +
                   "\"heats_run\": 17,\n" +
                   "\"name\": \"Lions &amp; Tigers, Round 1\"},\n" +
                 "{\"roundid\": 2,\n" +
                   "\"classid\": 2,\n" +
                   "\"class\": \"White\'s Wolves\",\n" +
                   "\"round\": 1,\n" +
                   "\"roster_size\": 17,\n" +
                   "\"passed\": 13,\n" +
                   "\"unscheduled\": 0,\n" +
                   "\"heats_scheduled\": 13,\n" +
                   "\"heats_run\": 13,\n" +
                   "\"name\": \"White\'s Wolves, Round 1\"},\n" +
                  "{\"roundid\": 3,\n" +
                   "\"classid\": 3,\n" +
                   "\"class\": \"Bears and Frèr\",\n" +
                   "\"round\": 1,\n" +
                   "\"roster_size\": 16,\n" +
                   "\"passed\": 2,\n" +
                   "\"unscheduled\": 0,\n" +
                   "\"heats_scheduled\": 4,\n" +
                   "\"heats_run\": 4,\n" +
                   "\"name\": \"Bears and Frèr, Round 1\"},\n" +
                  "{\"roundid\": 4,\n" +
                   "\"classid\": 4,\n" +
                   "\"class\": \"Webelos (\\\"Webes\",\n" +
                   "\"round\": 1,\n" +
                   "\"roster_size\": 16,\n" +
                   "\"passed\": 3,\n" +
                   "\"unscheduled\": 0,\n" +
                   "\"heats_scheduled\": 4,\n" +
                   "\"heats_run\": 4,\n" +
                   "\"name\": \"Webelos (\\\"Webes, Round 1\"},\n" +
                  "{\"roundid\": 5,\n" +
                   "\"classid\": 5,\n" +
                   "\"class\": \"Arrows &lt;&lt;--&lt;&lt;\",\n" +
                   "\"round\": 1,\n" +
                   "\"roster_size\": 16,\n" +
                   "\"passed\": 0,\n" +
                   "\"unscheduled\": 0,\n" +
                   "\"heats_scheduled\": 0,\n" +
                   "\"heats_run\": 0,\n" +
                   "\"name\": \"Arrows &lt;&lt;--&lt;&lt;, Round 1\"},\n" +
                  "{\"roundid\": 6,\n" +
                   "\"classid\": 6,\n" +
                   "\"class\": \"TheLastClass\",\n" +
                   "\"round\": 1,\n" +
                   "\"roster_size\": 0,\n" +
                   "\"passed\": 0,\n" +
                   "\"unscheduled\": 0,\n" +
                   "\"heats_scheduled\": 0,\n" +
                   "\"heats_run\": 0,\n" +
                   "\"name\": \"TheLastClass, Round 1\"},\n" +
                  "{\"roundid\": 8,\n" +
                   "\"classid\": 7,\n" +
                   "\"class\": \"Grand Finals\",\n" +
                   "\"aggregate\": true,\n" +
                   "\"round\": 1,\n" +
                   "\"roster_size\": 5,\n" +
                   "\"passed\": 5,\n" +
                   "\"unscheduled\": 5,\n" +
                   "\"heats_scheduled\": 0,\n" +
                   "\"heats_run\": 0,\n" +
                   "\"name\": \"Grand Finals, Round 1\"}]\n" +
      "}";
  await page.evaluate(function(json) { process_coordinator_poll_json(JSON.parse(json)); },
                      poll_result);

  assert.equal({'now-racing': [2],
                'ready-to-race': [],
                'not-yet-scheduled': [7, 5, 6, 8],
                'done-racing': [1, 3, 4]},
               await page.evaluate(function() { return g_rounds_layout; }));

  assert.equal(8, await page.evaluate(function() {
    return $(".scheduling_control_group .control_group.scheduling_control").length;
  }));
  assert.equal(8, await page.$$eval(".scheduling_control_group .control_group.scheduling_control",
                                    controls => { return controls.length; }));

  assert.equal([[],  // Headers row
                ["1", "282", "Willard Woolfolk", "3.977"],
                ["2", "207", "Blake Burling", "3.646"],
                ["3", "227", "Elliot Eastman", "2.295"],
                ["4", "222", "Dexter Dawes", "3.720"]],
               await page.evaluate(function() {
                 var table = [];
                 $("#now-racing-group .heat-lineup table tr").each(function(index, tr) {
                   var row = [];
                   $(tr).find("td").each(function(index, td) {
                     row.push($(td).text());
                   });
                   table.push(row);
                 });
                 return table;
               }));

  assert.equal("White's Wolves, Round 1",
               await page.$eval("#now-racing-group > div > h3", h3 => { return $(h3).text(); }));
  assert.equal("13 heats scheduled, 13 run.",
               await page.$eval("#now-racing-group p", p => { return $(p).text(); }));

  assert.equal([
    // Now racing
    "2", 0,
    // Master schedule
    "-1", 1,"Next Up",
    // Not yet scheduled
    "7", 2,"Schedule","Delete Round",
    "5", 0,  // No passed racers
    "6", 0,
    "8", 2,"Schedule","Delete Round",
    // Done racing
    "1", 2,"Make Changes","Repeat Round",
    "3", 2,"Make Changes","Repeat Round",
    "4", 2,"Make Changes","Repeat Round"],
               await page.evaluate(function() {
                 var buttons = [];
                 $("div.control_group[data-roundid]").each(function() {
                   buttons.push($(this).attr('data-roundid'));
                   buttons.push($(this).find("input[type='button']").length);
                   $(this).find("input[type='button']").each(function() {
                     buttons.push($(this).val());
                   });
                 });
                 return buttons;
               }));

  assert.equal(["Repeat Round", "Add New Rounds"],
               await page.evaluate(function() {
                 var buttons = [];
                 $("#supplemental-control-group input[type='button']").each(function() {
                   buttons.push($(this).val());
                 });
                 return buttons;
               }));
                 
  assert.includes("east", await page.$eval("#master-schedule-group .scheduling_control img",
                                           img => { return $(img).prop('src'); }));

  await open_scheduling_control_group("#master-schedule-group .scheduling_control");

  assert.includes("south", await page.$eval("#master-schedule-group .scheduling_control img",
                                            img => { return $(img).prop('src'); }));

  // A new poll response shouldn't close the control
  await page.evaluate(function(json) { process_coordinator_poll_json(JSON.parse(json)); },
                      poll_result);

  assert.includes("south", await page.$eval("#master-schedule-group .scheduling_control img",
                                            img => { return $(img).prop('src'); }));

  var master_schedule_group = await page.waitForSelector("#master-schedule-group .scheduling_control");
  master_schedule_group.click();

  await page.waitForFunction(() => { return $('#master-schedule-group .scheduling_control img').prop('src')
                                         .indexOf("east") > 0; });

  assert.includes("east", await page.$eval("#master-schedule-group .scheduling_control img",
                                           img => { return $(img).prop('src'); }));

  if (debugging) {
    console.log('End of test (would close browser if not debugging)');
  } else {
    await browser.close();
  }
});
