Array.prototype.forEach.call(JSON.parse(require('fs').read(phantom.libraryPath + "/../cookies.phantomjs")),
                             function(x) { phantom.addCookie(x); });

var assert = require('./assert');
var wait = require('./wait');
var system = require('system');
var page = require('webpage').create();

page.onConsoleMessage = function(msg) {
  console.log("From " + page.url + ": " + msg);
};

// TODO phantom.exit seems to allow whatever exit was invoked last to determine
// the process return value; not sure why.

phantom.onError = function(msg, trace) {
  var msgStack = ['PHANTOM ERROR: ' + msg];
  if (trace && trace.length) {
    msgStack.push('TRACE:');
    trace.forEach(function(t) {
      msgStack.push(' -> ' + (t.file || t.sourceURL) + ': ' +
                    t.line + (t.function ? ' (in function ' + t.function +')' : ''));
    });
  }
  console.error(msgStack.join('\n'));
  phantom.exit(1);
  // If execution continues after exit, maybe this helps?
  throw "Outtahere";
};

var args = system.args;
if (args.length < 2) {
  console.log("No root URL");
  phantom.exit(1);
}

var scriptName = args[0].substring(args[0].lastIndexOf('/') + 1);

var root = args[1];
if (root.substring(0, 'http://'.length) != 'http://') {
  root = 'http://' + root;
}
if (root.substring(root.length - 1) != '/') {
  root = root + '/';
}

window.assert = assert;
window.wait = wait;
window.system = system;
window.page = page;
window.root = root;
window.scriptName = scriptName;

window.phantom_testing = true;
