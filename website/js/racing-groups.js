'use strict';

function on_rule_change() {
  var val = $("input[type='radio'][name='form-groups-by']:checked").val();
  $.ajax('action.php',
         {type: 'POST',
          data: {action: 'partition.apply-rule',
                 rule: val,
                 cleanup: $("input#cleanup").is(':checked') ? 1 : 0},
          success: function(data) {
            console.log(data);
            poll_for_structure();
          }
         });
}
$(function() { $("input[type='radio'][name='form-groups-by']").on('change', on_rule_change); });

function on_use_subgroups_change(event, ignore) {
  if (ignore) return;

  $.ajax('action.php',
         {type: 'POST',
          data: {action: 'settings.write',
                 'do-use-subgroups': $("#use-subgroups").is(':checked') ? 1 : 0,
                 'do-use-subgroups-checkbox': true},
          success: function(data) {
            if (data.outcome.summary != 'success') {
              console.log(data);
            }
            poll_for_structure();
          }
         });
}
$(function() { $("#use-subgroups").on('change', on_use_subgroups_change); });

function on_cleanup_change() {
  if ($("input#cleanup").is(':checked')) {
    on_rule_change();  // Force a no-op apply-rule action with cleanup
  }
}
$(function() { $("input#cleanup").on('change', on_cleanup_change); });

function poll_for_structure() {
  $.ajax('action.php',
         {type: 'GET',
          data: {query: 'poll',
                 values: 'classes,partitions'},
          success: function(data) {
            process_polling_data(data);
          }
         });
}

$(function() { poll_for_structure(); });  // Draw initial structure

function process_polling_data(data) {
  populate_racing_groups(data, $("#use-subgroups").is(':checked'));
  populate_aggregate_modal(data);
  populate_aggregates(data.classes);
}

function populate_racing_groups(data, using_subgroups) {
  var all_groups = $("ul#all-groups");
  all_groups.children('li').not('#new-group').remove();
  var rule = $("input[type='radio'][name='form-groups-by']:checked").val();

  $("span.and-subgroups").toggleClass('hidden', !using_subgroups);
  $("p.instructions.custom").toggleClass('hidden', rule != 'custom');
  
  for (var i = 0; i < data.classes.length; ++i) {
    if (data.classes[i].hasOwnProperty('constituents') && data.classes[i].constituents.length > 0) {
      continue;
    }
    var cl = $("<li/>")
        .appendTo(all_groups)
        .addClass('group')
        .attr('data-classid', data.classes[i].classid)
        .attr('data-count', data.classes[i].count)
        .attr('data-nrounds', data.classes[i].nrounds)
        // data-constituent-of
        .append($("<p/>")
                .addClass('class-name')
                .text(data.classes[i].name)
                // label and count are float-right, so the first span is rightmost
                .append($("<span/>").text("group").addClass('label'))
                .append($("<span/>").text("(" + data.classes[i].count + ")").addClass('count'))
                .prepend($("<input type='button' value='Edit' class='edit-button'/>")
                         .on('click', on_edit_class)));

    var subgroups = $("<ul/>")
        .appendTo(cl)
        .addClass('subgroups');
    for (var j = 0; j < data.classes[i].subgroups.length; ++j) {
      var rankid = data.classes[i].subgroups[j].rankid;
      if (using_subgroups) {
        var subg = $("<li/>")
            .appendTo(subgroups)
            .addClass('subgroup')
            .attr('data-rankid', data.classes[i].subgroups[j].rankid)
            .attr('data-count', data.classes[i].subgroups[j].count)
            .append($("<p/>")
                    .addClass('rank-name')
                    .text(data.classes[i].subgroups[j].name)
                    .append($("<span/>").text("subgroup").addClass('label'))
                    .append($("<span/>").text("(" + data.classes[i].subgroups[j].count + ")")
                            .addClass('count'))
                    .prepend($("<input type='button' value='Edit' class='edit-button'/>")
                             .on('click', on_edit_rank)));
      } else {
        // If we're not showing subgroups, then append the UL for partitions directly to the LI for the class.
        var subg = cl;
      }
      if (rule == 'custom') {
        populate_partitions_in_subgroup(subg, rankid, data);
      }
    }
    if (rule == 'custom') {
      if (using_subgroups) {
        $("<li/>").appendTo(subgroups)
          .addClass('subgroup new-subgroup')
          .attr('data-classid', data.classes[i].classid)
          .on('click', on_add_subgroup)
          .append($("<p/>").text("New Subgroup"));
      }
    }
  }

  $("#new-group").appendTo(all_groups).toggleClass('hidden', rule != 'custom');
  
  make_groups_sortable();

  if (rule == 'custom') {
    make_partitions_draggable_droppable(using_subgroups);
  }
}

function populate_partitions_in_subgroup(subg, rankid, data) {
  var divs = $("<ul/>")
      .appendTo(subg)
      .addClass('partitions');
  // This can have lower complexity with an index, but in practice it's not worth the trouble
  for (var d = 0; d < data.partitions.length; ++d) {
    if (!data.partitions[d].rankids.includes(rankid)) {
      continue;
    }
    var div = $("<li/>")
        .appendTo(divs)
        .addClass('partition')
        .attr('data-partitionid', data.partitions[d].partitionid)
        .toggleClass('incomplete', data.partitions[d].rankids.length > 1)
        .append($("<p/>")
                .text(data.partitions[d].name)
                .append($("<span/>").text("(" + data.partitions[d].count + ")")
                        .addClass('count')));
  }
}

function make_groups_sortable() {
  $("ul#all-groups").sortable({
    items: "> li:not(#new-group)",
    stop: function(event, ui) {
      event.stopPropagation();

      var data = {action: 'class.order'};
      $("ul#all-groups > li").each(function(i) {
        data['classid_' + (i + 1)] = $(this).attr('data-classid');
      });
      console.log(data);
      
      $.ajax(g_action_url,
             {type: 'POST',
              data: data,
              success: function(data) {
                if (data.outcome.summary == 'success') {
                  poll_for_structure();
                }
              }
             });
    }
  });

  $("ul.subgroups").sortable({
    items: "> li.subgroup:not(.new-subgroup)",
    stop: function(event, ui) {
      event.stopPropagation();
      // Target is the ul
      var data = {action: 'rank.order'};
      $(event.target).find("li").each(function(i) {
        data['rankid_' + (i + 1)] = $(this).attr('data-rankid');
      });
      
      $.ajax(g_action_url,
             {type: 'POST',
              data: data,
              success: function(data) {
                if (data.outcome.summary == 'success') {
                  poll_for_structure();
                }
              }
             });
    }
  });
}

function make_partitions_draggable_droppable(using_subgroups) {
  $("li.partition").draggable({
    scope: 'custom-group',
    helper: 'clone',
    appendTo: 'body',
    opacity: 0.5,
    revert: 'invalid',
    // This allows dragging a partition to create its own group.  Dragging to a
    // group li (not its ul of subgroups) allows creating a subgroup within the
    // group.
    // connectToSortable: "ul#all-groups" 
  });

  $(using_subgroups ? "li.subgroup" : "li.group").droppable({
    scope: 'custom-group',
    accept: function(drag) {
      // Don't accept a child element for dropping
      return !$(this).has(drag).length; },
    greedy: true,
    drop: function(event, ui) {
      console.log($(ui.draggable).find('p').eq(0).text() + ' dropped on ' +
                  $(event.target).text());
      var draggable = $(ui.draggable);
      var div_id;
      if (draggable.is('[data-partitionid]')) {
        div_id = draggable.attr('data-partitionid');
      } else {
        console.log('Unrecognizable partition');
        return;
      }
      var droppable = $(event.target);
      var group_field;
      var group_id;
      var from_group_field;
      var from_group_id;
      if (droppable.attr('id') == 'new-group') {
        group_field = 'classid';
        group_id = -1;
      } else if (droppable.hasClass('group')) {
        group_field = 'classid';
        group_id = droppable.closest('li[data-classid]').attr('data-classid');
        from_group_field = 'classid';
        from_group_id = draggable.closest('li[data-classid]').attr('data-classid');
        if (from_group_id == group_id) {
          console.log('draggable closest classid=' +
                      draggable.closest('li[data-classid]').attr('data-classid'));
          return;
        }
      } else if (droppable.hasClass('new-subgroup')) {
        // Creating a new subgroup is expressed as moving to a known class
        group_field = 'classid';
        group_id = droppable.attr('data-classid');
      } else if (droppable.hasClass('subgroup')) {
        group_field = 'rankid';
        group_id = droppable.closest('li').attr('data-rankid');
        if (draggable.attr('data-rankid') == group_id) {
          console.log('draggable closest rankid=' + draggable.attr('data-rankid'));
          return;
        }
      } else {
        console.log('Droppable not recognized: '); console.log(droppable);
        return;
      }

      var data = {action: 'partition.move',
                  div_id: div_id,
                  group_field: group_field,
                  group_id: group_id,
                  cleanup: $("input#cleanup").is(':checked') ? 1 : 0};

      if (draggable.hasClass('incomplete')) {
        var from_li = draggable.parent().closest('li');
        console.log('from_li:');console.log(from_li);
        if (from_li.is('[data-classid]')) {
          data.from_group_field = 'classid';
          data.from_group_id = from_li.attr('data-classid');
        } else if (from_li.is('[data-rankid]')) {
          data.from_group_field = 'rankid';
          data.from_group_id = from_li.attr('data-rankid');
        } else {
          console.log("Incomplete not recognized:"); console.log(draggable);
        }
      }

      console.log(data);

      $.ajax('action.php',
             {type: 'POST',
              data: data,
              success: function(data) {
                console.log(data);
                if (data.outcome.summary == 'success') {
                  poll_for_structure();
                }
              }
             });
    },
  });
}

function populate_aggregates(classes) {
  var agg_groups = $("ul#aggregate-groups");
  agg_groups.empty();

  for (var i = 0; i < classes.length; ++i) {
    if (!(classes[i].hasOwnProperty('constituents') && classes[i].constituents.length > 0)) {
      continue;  // Aggregates only
    }
    var cl = $("<li/>")
        .appendTo(agg_groups)
        .addClass('aggregate')
        .attr('data-classid', classes[i].classid)
        .attr('data-count', classes[i].count)
        .attr('data-nrounds', classes[i].nrounds)
    // data-constituent-of
        .append($("<p/>")
                .attr('data-classid', classes[i].classid)
                .addClass('class-name')
                .text(classes[i].name)
                .append($("<span/>").text("agg group").addClass('label'))
                .prepend($("<input type='button' value='Edit' class='edit-button'/>")
                         .on('click', on_edit_class)));

    var constituents = $("<ul/>")
        .appendTo(cl)
        .addClass('constituents');
    // TODO poll.classes should report classid and classname in
    //  constituent-rank, obtainable from parallel constituents field?
    if (classes[i].hasOwnProperty('constituent-ranks')) {
      for (var v = 0; v < classes[i]['constituent-ranks'].length; ++v) {
        $("<li/>")
          .appendTo(constituents)
          .addClass('constituent-rank')
          .append($("<p/>").text(classes[i]['constituent-ranks'][v].name)
                  .append($("<span/>").text("constituent").addClass('label')));
      }
    } else {
      for (var v = 0; v < classes[i].constituents.length; ++v) {
        $("<li/>")
          .appendTo(constituents)
          .addClass('constituent')
          .append($("<p/>").text(classes[i].constituents[v].name)
                  .append($("<span/>").text("constituent").addClass('label')));
      }
    }

    for (var v = 0; v < classes[i].constituents.length; ++v) {
      // If a given class is a constituent of multiple aggregates, the last one wins
      $("li[data-classid='" + classes[i].constituents[v] + "']")
        .attr('data-constituent-of', classes[i].name);
    }
  }
}
