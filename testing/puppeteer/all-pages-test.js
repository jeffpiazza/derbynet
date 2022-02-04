const assert = require("./assert.js");

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

var role = 'RaceCoordinator';
var password = 'doyourbest';


var all_uris = [
  "about.php",
  "awards-editor.php",
  "awards-presentation.php",
  "checkin.php",
  "coordinator.php",
  // "export.php",
  "import-awards.php",
  "import-results.php",
  "import-roster.php",
  "index.php",
  "judging.php",
  "kiosk-dashboard.php",
  "login.php",
  "ondeck.php",
  "photo-thumbs.php",
  "racer-results.php",
  "racing-groups.php",
  "settings.php",
  "setup.php",
  "standings.php",
  "timer.php"
];

puppeteer.launch({devtools: debugging}).then(async browser => {
  const all_pages = await browser.pages();
  const page = all_pages[0];
  page.setViewport({width: 1200,
                    height: 1800});
  page.on('pageerror', (err) => {
    console.error(err);
    process.exit(1);
  });
  page.on('error', (err) => {
    console.error(err);
    process.exit(1);
  });

  for (u in all_uris) {
    var uri = all_uris[u];
    console.log("  " + "  " + "  " + uri);
    await page.goto(root + '/' + uri);
  }

  console.log("");
  console.log("  " + "  " + "  Logging in");
  await page.goto(root + '/login.php');
  await page.evaluate(function (role, password) {
    window.handle_login(role, password);
  }, role, password);
  await page.waitForNavigation();
  console.log("");

  for (u in all_uris) {
    var uri = all_uris[u];
    console.log("  " + "  " + "  " + uri);
    await page.goto(root + '/' + uri);
  }
  
  if (!debugging) {
    await browser.close();
  }
});
