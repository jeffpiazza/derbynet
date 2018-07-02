
// Polyfills for ECMAScript 2015
if (!String.prototype.endsWith) {
	String.prototype.endsWith = function(search, this_len) {
		if (this_len === undefined || this_len > this.length) {
			this_len = this.length;
		}
		return this.substring(this_len - search.length, this_len) === search;
	};
}
if (!String.prototype.startsWith) {
	String.prototype.startsWith = function(search, pos) {
		return this.substr(!pos || pos < 0 ? 0 : +pos, search.length) === search;
	};
}

// Returns true if 'plural' is likely a pluralization of 'singular'
function isLikelyPlural(singular, plural) {
  if (singular.length < plural.length &&
      plural.length <= singular.length + 2) {
    singular = singular.toLowerCase();
    plural = plural.toLowerCase();
    if (singular.length + 1 == plural.length
        && plural.endsWith('s')
        && plural.startsWith(singular)) {
      return true;
    }
    if (singular.length + 2 == plural.length
        && plural.endsWith('es')
        && plural.startsWith(singular)) {
      return true;
    }
    // Wolf and Wolves
    if (singular.length + 2 == plural.length
        && plural.endsWith('ves')
        && singular.endsWith('f')
        && plural.startsWith(singular.substr(0, singular.length - 1))) {
      return true;
    }
  }
  return false;
}


// Returns a string that is the likely canonicalized form of 'subject', given
// 'known', a list of known strings for the classname or subgroup.  If the
// return value differs from 'subject', the user should be alerted.
function likelyCanonicalized(subject, known) {
  for (var i = 0; i < known.length; ++i) {
    if (subject.toLowerCase() == known[i].toLowerCase()) {
      return known[i];
    }
    if (isLikelyPlural(subject, known[i]) || isLikelyPlural(known[i], subject)) {
      return known[i];
    }
  }
  return subject;
}
