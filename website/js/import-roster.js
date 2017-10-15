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
    onDropClassnameLabel(droppable);
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

function onDropClassnameLabel(droppable) {
  var classnames = collectClassNames(droppable.attr('data-column'));
  var existing_classes = all_classes();
  var nclasses = Object.keys(classnames).length;  // Before removing any existing names

  $('#existing_ranks').empty();
  if (existing_classes.length > 0) {
    $('#existing_ranks').append('<h4>Existing ' + $('[data-field="classname"]:first').text() + 's:</h4>');
    for (var cl in existing_classes) {
      var name = existing_classes[cl].name;
      $('#existing_ranks').append(document.createTextNode(name), '<br/>');
      delete classnames[name];
    }
  }

  var n_new_classes = Object.keys(classnames).length;

  $('#new_ranks').empty();
  if (n_new_classes > 0) {
    $('#new_ranks').append('<h4>New ' + $('[data-field="classname"]:first').text() + 's:</h4>');
    for (var cl in classnames) {
      $('#new_ranks').append(document.createTextNode(cl), '<br/>');
    }
  }

  $('#file-class-count').text(nclasses);
  $('#file-class-new-count').text(n_new_classes);
  $('#file-class-count-and-label').removeClass('hidden');
}

function onUndropClassnameLabel() {
  $("#new_ranks").empty();
}

// TODO: Something similar for ranks/subgroups
function collectClassNames(columnNumber) {
  var classnames = {};
  $('td.column' + columnNumber).each(function(index, elt) {
    // Don't count a header_row value
    if ($(elt).closest('.header_row').length == 0 &&
        $(elt).text().trim().length > 0) {
      classnames[$(elt).text()] = 1;
    }
  });
  return classnames;
}

$(function() {
  $('#import_button').click(function() {
    uploadTableRows('racer.import');
  });
});
