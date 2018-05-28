// assert module

function isEqual(expect, actual) {
  if (expect == actual) {
    return true;
  }
  if (typeof expect == 'object' && typeof actual == 'object') {
    if (expect.length != actual.length) {
      console.log("Fails by length:");
      console.log(JSON.stringify(expect));
      console.log(JSON.stringify(actual));
      return false;
    }
    for (var p in expect) {
      if (!isEqual(expect[p], actual[p])) {
        return false;
      }
    }
    return true;
  }
  console.log("Fails by equality:");
  console.log(expect);
  console.log(actual);
  return false;
}

exports.equal = function(expect, actual) {
  if (!isEqual(expect, actual)) {
    console.log("Expected: " + JSON.stringify(expect));
    console.log("Actual:   " + JSON.stringify(actual));
    process.exit(1);
  }
};

exports.notEqual = function(expect, actual) {
  if (expect == actual) {
    console.log("Expected notEqual: " + JSON.stringify(actual));
    process.exit(1);
  }
};

exports.includes = function(needle, haystack) {
  if (haystack.indexOf(needle) < 0) {
    console.log("Actual: " + haystack);
    console.log("Expected to contain " + needle);
    process.exit(1);
  }
};

