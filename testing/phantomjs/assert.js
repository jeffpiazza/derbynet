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
