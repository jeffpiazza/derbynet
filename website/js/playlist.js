function find_round(classid, roundno) {
  for (var i = 0; i < g_all_rounds.length; ++i) {
    if (g_all_rounds[i].classid == classid && g_all_rounds[i].round == roundno) {
      return g_all_rounds[i];
    }
  }
  return null;
}

function maybe_change_queue_message() {
  $("#top-of-queue").text(
    $("#queue-ul li").length == 0
      ? "Select rounds for playlist"
      : "Drag to reorder");
}

function on_racing_scene_change() {
  $.ajax('action.php',
         {type: 'POST',
          data: {action: 'settings.write',
                 racing_scene: $("#racing-scene").val()}
         });
}

function setup_racing_scene_control(all_scenes, current_scene) {
  $("#racing-scene").empty();
  var first_selection = 0;
  for (var i = 0; i < all_scenes.length; ++i) {
    var scene = all_scenes[i];
    $("#racing-scene").append($("<option/>")
                              .attr('value', scene.sceneid)
                              .text(scene.name));
    if (scene.sceneid == current_scene) {
      first_selection = i;
    }
  }
  $("#racing-scene").append($("<option/>")
                            .attr('value', -1)
                            .text("(New scene)"));
  $("#racing-scene")
    .val(all_scenes[first_selection].sceneid)
    .trigger('change')
    .on('change', on_racing_scene_change);
}

function on_open_queue_entry() {
  var li = $(this).closest('li');
  var closed = li.find(".collapsible").css("display") == "none";

  // Close everybody else
  $("img.triangle").attr('src', 'img/triangle_east.png');
  $(".collapsible").slideUp(200, function() {
    $(this).closest('li').find('span.after-action').removeClass('hidden');
  });

  // ... and open this one if it was closed
  if (closed) {
    li.find("img.triangle").attr('src', 'img/triangle_south.png');
    li.find(".collapsible").slideDown(200, function() {
      $(this).closest('li').find('span.after-action').addClass('hidden');
    });
  }
}

function on_change_times_per_lane() {
  on_queue_entry_update($(this).closest('li'));
}

function update_queue_entry_format(after_sel) {
  var li = after_sel.closest('li');

  li.find('p.gap').toggleClass('squeeze', after_sel.val() == -1);
  var selected = after_sel.find('option:selected');
  if (selected.val() == 0) {
    li.find('span.after-action').html("&nbsp;");
  } else {
    li.find('span.after-action').text(selected.text());
  }
}

function on_change_sceneid_at_finish() {
  var after_sel = $(this);
  var li = after_sel.closest('li');

  update_queue_entry_format(after_sel);
  
  on_queue_entry_update(li);
}

function on_queue_entry_update(li) {
  var queueid = li.attr('data-queueid');
  var after = li.find('#after_' + queueid).val();

  var entry = g_queue[li.index()];
  entry.n_times_per_lane = li.find('#reps_' + queueid).val();
  entry.sceneid_at_finish = after;
  entry.continue_racing = (after == 0 ? 0 : 1);

  $.ajax('action.php',
         {type: 'POST',
          data: {action: 'queue.update',
                 queueid: queueid,
                 n_times_per_lane: entry.n_times_per_lane,
                 sceneid_at_finish: entry.sceneid_at_finish,
                 continue_racing: entry.continue_racing},
          success: function() {
          }
         });
}

function on_remove_queue_entry() {
  var li = $(this).closest('li');
  $.ajax('action.php',
         {type: 'POST',
          data: {action: 'queue.remove',
                 queueid: g_queue[li.index()].queueid},
          success: function(data) {
            g_queue.splice(li.index(), 1);
            li.remove();
            maybe_change_queue_message();
            build_rounds(g_queue, g_classes, g_all_rounds);
          }
         });

  return false;
}

function on_reorder(ul) {
  var data = {action: 'queue.order'};
  $(ul).each(function(i) {
    data['queueid_' + (i + 1)] = $(this).attr('data-queueid');
  });

  $.ajax('action.php',
         {type: 'POST',
          data: data});
}

function on_add_round_to_queue(evt) {
  var li = $(this).closest('li');
  if (li.attr('data-roundid')) {
    add_round_to_queue(li.attr('data-classid'), li.attr('data-round'), {});
  } else {
    show_create_roster_dialog(li.attr('data-classid'), li.attr('data-round'));
  }
}

function show_create_roster_dialog(classid, roundno) {
  // roster.new action:
  // For a follow-on round (round >= 2): $_POST['roundid'] + $_POST['top'] + ['bucketed']
  // For a first aggregate (round = 1 and no roundid), classid for the aggregate class, plus top + bucketed
  classid = parseInt(classid);
  var cl = g_classes[classid];

  $("#new-roster-modal .hidable").addClass('hidden');
  if (roundno == 1) {
    $("#new-roster-modal .group-buckets").removeClass('hidden');
  } else if (g_use_subgroups) {
    $("#new-roster-modal .subgroup-buckets").removeClass('hidden');
  } else {
    $("#new-roster-modal .no-buckets").removeClass('hidden');
  }

  show_modal("#new-roster-modal", function (event) {
    close_modal("#new-roster-modal");
    // For roundno >= 2, the previous round may not be defined, so roster
    // contruction will have to figure out the roundid only at the very last
    // moment.
    add_round_to_queue(classid, roundno,
                       {top: $("#new_roster_top").val(),
                        bucketed:
                          roundno == 1 ? $("#bucketed_groups").is(':checked')
                          : g_use_subgroups ? $("#bucketed_subgroups").is(':checked')
                          : false}
                      );
    return false;
  });
}

function add_round_to_queue(classid, round, roster_params) {
  $.ajax('action.php',
         {type: 'POST',
          data: {action: 'queue.new',
                 classid: classid,
                 round: round,
                 top: roster_params ? roster_params.top : 0,
                 bucketed: roster_params ? roster_params.bucketed : false,
                 n_times_per_lane: 1,
                 continue_racing: 1},
          success: function(data) {
            var q = data.getElementsByTagName('queue-entry');
            if (q) {
              var entry = JSON.parse(q[0].textContent);
              entry.round = parseInt(entry.round);
              entry.seq = parseInt(entry.seq);
              g_queue.push(entry);
              $('#queue-ul').append(make_queue_entry_li(entry, g_all_scenes));
              maybe_change_queue_message();
              // TODO Maybe a new round
              build_rounds(g_queue, g_classes, g_all_rounds);
            }
          }
         });
}

// Returns the new li.  entry = {queueid, roundname, n_times_per_lane, sceneid_at_finish, continue_racing}
function make_queue_entry_li(entry, all_scenes) {
  var reps_id = 'reps_' + entry.queueid;
  var after_id = 'after_' + entry.queueid;

  var after_sel = $("<select/>")
      .attr('id', after_id)
      .append($("<option>Stop</option>").attr('value', 0))
      .append($("<option>Start Next Round</option>").attr('value', -1));
  $.each(all_scenes, function(i, v) {
    after_sel.append($("<option/>")
                     .attr('value', v.sceneid)
                     .text(v.name + " Scene"));
  });

  var reps = $("<select/>").attr('id', reps_id)
      .append("<option>1</option>")
      .append("<option>2</option>")
      .append("<option>3</option>")
      .append("<option>4</option>")
      .append("<option>5</option>")
      .append("<option>6</option>")
      .val(entry.n_times_per_lane)
      .on('change', on_change_times_per_lane);
  
  var li = $("<li class='queue'/>")
      .attr('data-queueid', entry.queueid)
      .attr('data-classid', entry.classid)
      .append($("<div class='queued-round'/>")
              .append($("<p/>")
                      .text(entry.roundname)
                      .prepend("<img class='triangle' src='img/triangle_east.png'/>")
                      .append($("<input type='button' class='queue-remove' value=' x '/>")
                              .on('click', on_remove_queue_entry))
                     )
              .append($("<div class='collapsible'/>")
                      .css('display', 'none')
                      .append($("<label>Runs per lane:</label>").attr('for', reps_id))
                      .append(reps)
                      .append($("<label>After racing:</label>").attr('for', after_id))
                      .append(after_sel)))
      .append("<p class='gap'><span class='after-action'/></p>")
      .on('click', on_open_queue_entry);

  if (entry.sceneid_at_finish > 0) {
    after_sel.val(entry.sceneid_at_finish);
  } else if (parseInt(entry.continue_racing)) {
    after_sel.val(-1);
  } else {  // stop
    after_sel.val(0);
  }
  after_sel.on('change', on_change_sceneid_at_finish);

  update_queue_entry_format(after_sel);

  mobile_select(reps);
  mobile_select(after_sel);
  return li;
}

function build_rounds(queue, classes, rounds) {
  $('#rounds-ul').empty();

  var highest_round = 0;  // Highest round overall
  $.each(queue, function(i, entry) {
    // entry = {classid, round}
    entry.round = parseInt(entry.round);
    entry.seq = parseInt(entry.seq);
      if (entry.round > highest_round) {
        highest_round = entry.round;
      }
  });
  $.each(rounds, function(i, entry) {
    entry.round = parseInt(entry.round);
    if (entry.round > highest_round) {
      highest_round = entry.round;
    }
  });

  var first_ever = true;
  for (var r = 1; r <= highest_round + 1; ++r) {
    var first_in_round = true;
    if (first_ever) {
      first_in_round = false;
      first_ever = false;
    }
    $.each(classes, function(classid, cl) {
      var in_queue = false;
      $.each(queue, function(qi, q) {
        if (q.classid == classid && q.round == r) {
          in_queue = true;
          return false;
        }
      });
      if (!in_queue) {
        var round_exists = false;
        $.each(rounds, function(ri, round) {
          if (round.classid == classid && round.round == r) {
            round_exists = round.roundid;
            return false;
          }
        });
        if (first_in_round) {
          $("#rounds-ul").append("<div class='spacer'/>");
          first_in_round = false;
        }
        $("#rounds-ul").append(
          $("<li class='draggable width200'/>")
            .text(cl.class + ', Round ' + r)  // TODO roundname
            .attr('data-classid', classid)
            .attr('data-round', r)
            .attr('data-roundid', function() {
              if (round_exists) {
                return round_exists;
              }
            })
            .append($("<input type='button' value='+'/>").on('click', on_add_round_to_queue))
        );
      }
    });
  }
}
  
$(function() {
  setup_racing_scene_control(g_all_scenes, g_current_racing_scene);
  // TODO Prevent scheduling rounds before their precursors, or constituents.

  $("#queue-ul").sortable({
    revert: true,

    update: function(event, ui) {
      // Clicking on a list item wants to open it (on_open_queue_entry); if
      // we're dragging, cancel that.
      var target = $(event.target);
      target.find("img.triangle").attr('src', 'img/triangle_east.png');
      target.find('.collapsible').css('display', 'none');
      target.find('span.after-action').removeClass('hidden');
    },
    over: function(event, ui) {
      $("#queue-ul").addClass('hover');
    },
    out: function(event, ui) {
      $("#queue-ul").removeClass('hover');
    },
    stop: function(event, ui) {
      on_reorder($("#queue-ul"));
    }
  });
});
