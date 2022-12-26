'use strict';
// Code to populate and drive the "add class"/"add aggregate class" modal.


// For the aggregate part of the "Add Class" modal, add a flipswitch for each
// existing group in div#constituent-classes, and a flipswitch for each existing
// subgroup for potential aggregation in div#constituent-subgroups
function populate_aggregate_modal(data) {
  var classes = data.classes;
  $("#constituent-classes").empty();
  $("#constituent-subgroups").empty();
  for (var i = 0; i < classes.length; ++i) {
    var cl = classes[i];
    $("<div class='flipswitch-div'/>")
      .appendTo("#constituent-classes")
      .append($("<label class='constituent-label'/>")
              .attr('for', 'constituent_' + cl.classid)
              .text(cl.name))
      .append($("<input type='checkbox' class='flipswitch'/>")
              .attr('id', 'constituent_' + cl.classid)
              .attr('name', 'constituent_' + cl.classid));

    var ranks = cl.subgroups;
    for (var ri = 0; ri < ranks.length; ++ri) {
      var r = ranks[ri];
      $("<div class='flipswitch-div'/>")
        .appendTo("#constituent-subgroups")
        .append($("<label class='constituent-label'/>")
                .attr('for', 'rankid_' + r.rankid)
                .text(r.name))
        .append($("<input type='checkbox' class='flipswitch'/>")
                .attr('id', 'rankid_' + r.rankid)
                .attr('name', 'rankid_' + r.rankid));
    }
  }
  // Decorate all the flipswitches
  flipswitch($("#constituent-div input.flipswitch"));
  $("#constituent-div input[type='checkbox']").on('change', maybe_enable_aggregate_create);
}

// When the "aggregate-by" checkbox changes, slide to hide or show
// constituent-rounds div
function on_aggregate_by_change() {
  $("#constituent-classes").css('margin-left',
                                $("#aggregate-by-checkbox").is(':checked') ? '-500px' : '0px');
  maybe_enable_aggregate_create();
}

// Add a new organic (non-aggregate) class
function on_add_class() {
  $("#add_class_modal input[name='name']").val("");
  $(".aggregate-only").addClass("hidden");
  $("#add_class_modal input[type='submit']").prop('disabled', false);

  show_modal("#add_class_modal", function () {
    close_add_class_modal();
    $.ajax(g_action_url,
           {type: 'POST',
            data: 'action=class.add&' +
            $("#add-class-name").serialize() + '&' +
            $("#add-class-ntrophies").serialize(),
            success: function (data) {
              $("#cleanup")
                .prop('checked', false)
                .trigger('change', /*synthetic*/true);
              poll_for_structure();
            }});

    return false;
  });
}
$(function() { $("#new-group").on('click', on_add_class); });

// Don't allow creating an aggregate class when there are fewer than two
// constituents selected.  This gets attached as an on('changed') function to
// each of the checkboxes for constituents.
function maybe_enable_aggregate_create() {
  $("#add_class_modal input[type='submit']")
    .prop('disabled',
          ($("#aggregate-by-checkbox").is(':checked')
           ? $("#constituent-subgroups input[type='checkbox']:checked").length
           : $("#constituent-classes input[type='checkbox']:checked").length)
          < 2);
}

function on_add_aggregate() {
  $("#add_class_modal input[name='name']").val("");
  $(".aggregate-only").removeClass('hidden');
  $("#aggregate-by-div").toggleClass('hidden', ! $("#use-subgroups").is(':checked'));

  $("#constituent-classes input[type='checkbox']")
    .prop('checked', false)
    .trigger('change', /*synthetic*/true);

  maybe_enable_aggregate_create();

  show_modal("#add_class_modal", function () {
    close_add_class_modal();
    $.ajax(g_action_url,
           {type: 'POST',
            data:  'action=class.add&' +
            $("#add-class-name").serialize() + '&' +
            $("#add-class-ntrophies").serialize() + '&' +
            ($("#aggregate-by-checkbox").is(':checked')
             ? $("#constituent-subgroups input").serialize()
             : $("#constituent-classes input").serialize()),
            success: function (data) {
              poll_for_structure();
            }});
    return false;
  });
}

function close_add_class_modal() {
  close_modal("#add_class_modal");
}

$(function() { $("#add-aggregate-button").on('click', on_add_aggregate); });

function on_add_partition(event) {
  $("#add_partition_modal input[name='name']").val("");
  show_modal("#add_partition_modal", function() {
    close_add_partition_modal();
    $.ajax(g_action_url,
           {type: 'POST',
            data: $("#add_partition_modal form").serialize(),
            success: function(data) {
              $("#cleanup")
                .prop('checked', false)
                .trigger('change', /*synthetic*/true);
              poll_for_structure();
            }});
    return false;
  });
}

function close_add_partition_modal() {
  close_modal("#add_partition_modal");
}

$(function() { $("#add-partition-button").on('click', on_add_partition); });
