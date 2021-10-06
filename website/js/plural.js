
// Contents of this file repackaged and converted to javascript
// from https://github.com/mjackson/plural
//  * @author  Michael J. I. Jackson <mjijackson@gmail.com>

function plural(word) {
  var plural = [
    [ /(matr|vert|ind)(ix|ex)$/i   , '$1ices'],    // matrix, vertex, index
    [ /(ss|sh|ch|x|z)$/i           , '$1es'],      // sibilant rule (no ending e)
    [ /([^aeiou])o$/i              , '$1oes'],     // -oes rule
    [ /([^aeiou]|qu)y$/i           , '$1ies'],     // -ies rule
    [ /sis$/i                      , 'ses'],       // synopsis, diagnosis
    [ /(m|l)ouse$/i                , '$1ice'],     // mouse, louse
    [ /(t|i)um$/i                  , '$1a'],       // datum, medium
    [ /([li])fe?$/i                , '$1ves'],     // knife, life, shelf
    [ /(octop|vir|syllab)us$/i     , '$1i'],       // octopus, virus, syllabus
    [ /(ax|test)is$/i              , '$1es'],      // axis, testis
    [ /([a-rt-z])$/i               , '$1s']        // not ending in s
  ];
  var irregular = {
    // words that don't follow any pluralization rules
    'bus':           'busses',
    'child':         'children',
    'man':           'men',
    'person':        'people',
    'quiz':          'quizzes',
    // words whose singular and plural forms are the same
    'equipment':     'equipment',
    'fish':          'fish',
    'information':   'information',
    'money':         'money',
    'moose':         'moose',
    'news':          'news',
    'rice':          'rice',
    'series':        'series',
    'sheep':         'sheep',
    'species':       'species',
  }

  word = word.trim();
  if (irregular.hasOwnProperty(word)) {
    return irregular[word];
  }
  for (var i = 0; i < plural.length; ++i) {
    if (word.match(plural[i][0])) {
      return word.replace(plural[i][0], plural[i][1]);
    }
  }
  return word;
}

