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
  if (draggable.attr('data-field') == 'partition') {
    onDropPartitionLabel(droppable.attr('data-column'));
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
  if (draggable.attr('data-field') == 'partition') {
    onUndropPartitionLabel();
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

  var partition_column_target = $("div[data-field='partition']").closest('.label_target');
  if (partition_column_target.length > 0) {
    calculateNewPartitions(partition_column_target.attr('data-column'));
  }
}

function show_class_details() {
  show_modal('#new_partitions_modal', function() {
  });
}

// Counts the number of table rows (actually, first-column td elements) that
// aren't in a header row.
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

function onDropPartitionLabel(column_number) {
  calculateNewPartitions(column_number);
}

function calculateNewPartitions(column_number) {
  // partitions_to_import is an object, with class names from the csv as properties.
  // existing_partitions is an array of strings
  var partitions_to_import = collectPartitionNames(column_number);
  var existing_partitions = all_partitions();
  var partition_label = $("#partition-label").val();

  $('#existing_partitions_div').empty();
  if (existing_partitions.length > 0) {
    $('#existing_partitions_div').append('<h4>Existing ' + plural(partition_label) + ':</h4>');
    for (var cl in existing_partitions) {
      var name = existing_partitions[cl];
      $('#existing_partitions_div').append(document.createTextNode(name), '<br/>');
      delete partitions_to_import[name];
    }
  }

  var n_new_partitions = 0;

  $('#new_partitions_div').empty();
  for (var cl in partitions_to_import) {
    if (partitions_to_import[cl] == 1) {
      $('#new_partitions_div').append(document.createTextNode(cl), '<br/>');
      ++n_new_partitions;
    }
  }

  if (n_new_partitions > 0) {
    $('#new_partitions_div').prepend('<h4>New ' + plural(partition_label) + ':</h4>');
  }

  $('#file-class-count').text(existing_partitions.length + n_new_partitions);
  $('#file-class-new-count').text(n_new_partitions);
  $('#file-class-count-and-label').removeClass('hidden');
}

function onUndropPartitionLabel() {
  $("#new_partitions_div").empty();
}

// Maybe not what you think: returns an object with each class name being the
// name of a property.
function collectPartitionNames(columnNumber) {
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

// The user changes the name of the partition label, e.g., calling it something
// other than "Den".
function on_partition_label_change() {
  $("div[data-field='partition']").text($("#partition-label").val());
  $("#existing-partition-label").text($("#partition-label").val() + 's');
  $.ajax('action.php',
         {type: 'POST',
          data: {action: 'settings.write',
                 'partition-label': $("#partition-label").val()}
         });
}
$(function() {
  $("#partition-label").on('keyup mouseup', on_partition_label_change);
});

$(function() {
  $('#import_button').click(function() {
    uploadTableRows('racer.import');
  });
});
