var wait = require('./wait');
var system = require('system');
var page = require('webpage').create();


var args = system.args;
if (args.length < 2) {
  console.log("No root URL");
  phantom.exit(1);
}

var root = args[1];
if (root.substring(0, 'http://'.length) != 'http://') {
  root = 'http://' + root;
}
if (root.substring(root.length - 1) != '/') {
  root = root + '/';
}

var pwdfile = system.env['PASSWORDS_FILE'];
if (!pwdfile) {
  pwdfile = phantom.libraryPath + '/../default.passwords';
}
var pwd;

require('fs').read(pwdfile).split('\n').forEach(function(line) {
  var m = line.match(/^RaceCoordinator:(.*)/);
  if (m) {
    pwd = m[1];
  }
});

page.open(root + 'login.php', function(status) {
  page.evaluate(function(pwd) {
    handle_login('RaceCoordinator', pwd);
  }, pwd);
  console.log("  " + "  " + "  login.js: ajax login request sent");
  // After logging in, the login page redirects to the index page, indicating succss.
  wait.for(function() {
    return page.url.substring(page.url.length - 'index.php'.length) == 'index.php';
  },
           function() {
             require('fs').write(phantom.libraryPath + "/../cookies.phantomjs",
                                 JSON.stringify(page.cookies), "w");
             console.log("  " + "  " + "  login.js: session cookie captured");
             phantom.exit(); });
});

