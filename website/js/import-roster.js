// This file customizes behavior from import-csv.js

// The first-last field (one column has both first names and last names) takes a
// little special handling.  Note that the 'submit' button becomes active only
// if all the 'required' labels are placed.)
//
//
// If first-last is assigned to a column, then neither firstname nor lastname
// should be available (visible), nor are they now 'required' fields.
//
// If either firstname or lastname is assigned to a column, then first-last
// becomes unavailable and not required.
function onDrop(draggable, droppable) {
  if (draggable.attr('data-field') == 'classname') {
    onDropClassnameLabel(droppable.attr('data-column'));
  } else if (draggable.attr('data-field') == 'first-last') {
    $('.field[data-field="firstname"], .field[data-field="lastname"]').addClass('hidden').removeClass('required');
  } else if (draggable.attr('data-field') == 'lastname' ||
             draggable.attr('data-field') == 'firstname') {
    $('.field[data-field="first-last"]').addClass('hidden').removeClass('required');
  }
}

// Similarly for undrop: if BOTH firstname and lastname fields are returned to
// the palette, then first-last becomes available again (and shown as required).
//
// Returning first-last to the palette re-enables both firstname and lastname.
function onUndrop(draggable) {
  if (draggable.attr('data-field') == 'classname') {
    onUndropClassnameLabel();
  } else if (draggable.attr('data-field') == 'first-last') {
    $('.field[data-field="firstname"], .field[data-field="lastname"]').removeClass('hidden').addClass('required');
  } else if (draggable.attr('data-field') == 'lastname' ||
             draggable.attr('data-field') == 'firstname') {
    if ($('.field[data-field="firstname"], .field[data-field="lastname"]').closest('[data-home]').length == 2) {
      $('.field[data-field="first-last"]').removeClass('hidden').addClass('required');
    }
  }
}

function onFileContentLoaded(file) {
  // TODO 'Encoding chosen: ' + $('input:radio[name="encoding-select"]:checked').val());
  $('#state-of-play').removeClass('hidden');
  $('#file-stats').removeClass('hidden');
  $('#file-name').text(file.name);
  $('#file-racer-count').text(countNewRacers());
  $('#file-class-count-and-label').addClass('hidden');
  $('#class-counts-button').on('click', function() { show_class_details(); });
}

function onHeaderRowToggle() {
  $('#file-racer-count').text(countNewRacers());

  var classname_column_target = $("div[data-field=classname]").closest('.label_target');
  if (classname_column_target.length > 0) {
    calculateNewClasses(classname_column_target.attr('data-column'));
  }
}

function show_class_details() {
  show_modal('#new_ranks_modal', function() {
  });
}

function countNewRacers() {
  var racers = 0;
  $('td.column0').each(function(index, elt) {
    if ($(elt).closest('.header_row').length == 0 &&
        $(elt).text().trim().length > 0) {
      ++racers;
    }
  });
  return racers;
}

function onDropClassnameLabel(column_number) {
  calculateNewClasses(column_number);
}

function calculateNewClasses(column_number) {
  // classnames_to_import is an object, with class names from the csv as properties
  // existing_classes is an array of strings
  var classnames_to_import = collectClassNames(column_number);

  var all = all_classes();
  var n_existing_classes = all.length;
  var existing_classes = [];
  for (i = 0; i < n_existing_classes; ++i) {
    existing_classes[i] = all[i].name;
  }

  var canonicalized = {};
  var combined_classes = existing_classes.slice();  // Copy the array
  for (var cl in existing_classes) {
    canonicalized[existing_classes[cl]] = existing_classes[cl];
  }
  for (var new_cl in classnames_to_import) {
    var canon = likelyCanonicalized(new_cl, combined_classes);
    canonicalized[new_cl] = canon;
    if (new_cl == canon) {
      combined_classes.push(new_cl);
    } else {
      classnames_to_import[new_cl] = 0;  // Name changed
    }
  }
  rewriteClassnames(column_number, canonicalized);

  $('#existing_ranks').empty();
  if (existing_classes.length > 0) {
    $('#existing_ranks').append('<h4>Existing ' + $('[data-field="classname"]:first').text() + 's:</h4>');
    for (var cl in existing_classes) {
      var name = existing_classes[cl];
      $('#existing_ranks').append(document.createTextNode(name), '<br/>');
      delete classnames_to_import[name];
    }
  }

  var n_new_classes = 0;

  $('#new_ranks').empty();
  for (var cl in classnames_to_import) {
    if (classnames_to_import[cl] == 1) {
      $('#new_ranks').append(document.createTextNode(cl), '<br/>');
      ++n_new_classes;
    }
  }

  if (n_new_classes > 0) {
    $('#new_ranks').prepend('<h4>New ' + $('[data-field="classname"]:first').text() + 's:</h4>');
  }
  $('#file-class-count').text(n_existing_classes + n_new_classes);
  $('#file-class-new-count').text(n_new_classes);
  $('#file-class-count-and-label').removeClass('hidden');
}

function onUndropClassnameLabel() {
  $("#new_ranks").empty();
}

// Maybe not what you think: returns an object with class name *properties*!
//
// TODO: Something similar for ranks/subgroups
function collectClassNames(columnNumber) {
  var classnames = {};
  $('td.column' + columnNumber).each(function(index, elt) {
    // Don't count a header_row value
    if ($(elt).closest('.header_row').length == 0 &&
        $(elt).text().trim().length > 0) {
      classnames[$(elt).text().trim()] = 1;
    }
  });
  return classnames;
}

function rewriteClassnames(columnNumber, canonicalized) {
  $('td.column' + columnNumber).each(function(index, elt) {
    if ($(elt).closest('.header_row').length == 0 &&
        $(elt).text().trim().length > 0) {
      $(elt).text(canonicalized[$(elt).text().trim()]);
    }
  });
}

$(function() {
  $('#import_button').click(function() {
    uploadTableRows('racer.import');
  });
});
