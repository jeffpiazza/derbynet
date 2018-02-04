// assert module

exports.success = function (status) {
  if (status != 'success') {
    console.log('Page load fails: ' + status);
    phantom.exit(1);
  }
};

exports.equal = function(expect, actual) {
  if (expect != actual) {
    console.log("Expected: " + expect);
    console.log("Actual: " + actual);
    phantom.exit(1);
  }
};

exports.notEqual = function(expect, actual) {
  if (expect == actual) {
    console.log("Expected notEqual: " + actual);
    phantom.exit(1);
  }
};

exports.includes = function(needle, haystack) {
  if (haystack.indexOf(needle) < 0) {
    console.log("Actual: " + haystack);
    console.log("Expected to contain " + needle);
    phantom.exit(1);
  }
};

