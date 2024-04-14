function find_round(classid, roundno) {
  if (g_all_rounds.hasOwnProperty(classid)) {
    for (var i = 0; i < g_all_rounds[classid].length; ++i) {
      if (g_all_rounds[classid][i].round == roundno) {
        return g_all_rounds[classid][i];
      }
    }
  }
  return null;
}

// The <select> for after racing includes options for:
//  - Stop racing (val = 0)
//  - Start next round (val = -1)
//  - A sceneid (val > 0)
//
// sel is a jquery selector for the <select> entry,
// all_scenes a list of scene entries
function build_after_action_select(sel, all_scenes) {
  sel = $(sel);
  sel.empty()
    .append($("<option>Stop</option>").attr('value', 0))
    .append($("<option>Start Next Round</option>").attr('value', -1));
  $.each(all_scenes, function(i, v) {
    sel.append($("<option/>")
               .attr('value', v.sceneid)
               .text(v.name + " Scene"));
  });
  mobile_select_refresh(sel);
}

// Set entry.sceneid_at_finish, entry.continue_racing from
// <select> element identified by sel.
function read_after_action_select(sel, entry) {
  var after = $(sel).val();
  entry.sceneid_at_finish = after;
  // continue_racing is only meaningful if sceneid_at_finish <= 0.  0 means stop
  // (don't continue racing), -1 means continue to next round (continue racing).
  entry.continue_racing = (after == 0 ? 0 : 1);
}

function maybe_change_playlist_message() {
  $("#top-of-queue").text(
    $("#queue-ul li").length == 0
      ? "Select rounds for playlist"
      : "Playlist: Drag entries to reorder");
}

function on_racing_scene_change() {
  $.ajax('action.php',
         {type: 'POST',
          data: {action: 'settings.write',
                 racing_scene: $("#racing-scene").val()},
          success: function(data) {
            $("#racing-scene-psa").toggleClass('hidden', $("#racing-scene").val() >= 0);
          }
         });
}

// This populates the control for the "racing scene," the scene to activate
// while racing is actually underway.
function setup_racing_scene_control(all_scenes, current_scene) {
  $("#racing-scene").empty();
  var first_selection = -1;
  $("#racing-scene").append($("<option/>")
                            .attr('value', -1)
                            .text("(None)"));
  for (var i = 0; i < all_scenes.length; ++i) {
    var scene = all_scenes[i];
    $("#racing-scene").append($("<option/>")
                              .attr('value', scene.sceneid)
                              .text(scene.name));
    if (scene.sceneid == current_scene) {
      first_selection = scene.sceneid;
    }
  }
  $("#racing-scene")
    .val(first_selection)
    .trigger('change')
    .on('change', on_racing_scene_change);
  $("#racing-scene-psa").toggleClass('hidden', $("#racing-scene").val() >= 0);
}

function on_open_playlist_entry(evt) {
  if ($(evt.target).is('select')) {
    // This prevents collapsing the collapsible part in response to a click on
    // one of the <select> elements.
    return true;
  }

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

function on_change_bucket_limit() {
  var li = $(this).closest('li');
  g_queue[li.index()].bucket_limit = $(this).val();
  on_playlist_entry_update(li);
}

function on_change_bucketed() {
  var li = $(this).closest('li');
  g_queue[li.index()].bucketed = $(this).is(':checked') ? 1 : 0;
  on_playlist_entry_update(li);
}

function on_change_times_per_lane() {
  var li = $(this).closest('li');
  g_queue[li.index()].n_times_per_lane = $(this).val();
  on_playlist_entry_update(li);
}

function on_change_sceneid_at_finish() {
  var li = $(this).closest('li');
  var entry = g_queue[li.index()];
  read_after_action_select(this, entry);
  update_playlist_entry_format($(this));

  on_playlist_entry_update(li);
}

function update_playlist_entry_format(after_sel) {
  var li = after_sel.closest('li');

  li.find('p.gap').toggleClass('squeeze', after_sel.val() == -1);
  var selected = after_sel.find('option:selected');
  if (selected.val() == 0) {
    li.find('span.after-action').html("&nbsp;");
  } else {
    li.find('span.after-action').text(selected.text());
  }
}

function on_playlist_entry_update(li) {
  var queueid = li.attr('data-queueid');
  var entry = g_queue[li.index()];

  $.ajax('action.php',
         {type: 'POST',
          data: {action: 'playlist.edit',
                 queueid: queueid,
                 top: entry.bucket_limit,
                 bucketed: entry.bucketed,
                 n_times_per_lane: entry.n_times_per_lane,
                 sceneid_at_finish: entry.sceneid_at_finish,
                 continue_racing: entry.continue_racing},
          success: function() {
          }
         });
}

function on_remove_playlist_entry() {
  var li = $(this).closest('li');
  $.ajax('action.php',
         {type: 'POST',
          data: {action: 'playlist.delete',
                 queueid: g_queue[li.index()].queueid},
          success: function(data) {
            g_queue.splice(li.index(), 1);
            li.remove();
            maybe_change_playlist_message();
            build_rounds(g_queue, g_classes);
          }
         });

  return false;
}

function on_reorder(ul) {
  var data = {action: 'playlist.order'};
  $(ul).find('li').each(function(i) {
    data['queueid_' + (i + 1)] = $(this).attr('data-queueid');
  });
  $.ajax('action.php',
         {type: 'POST',
          data: data,
          success: function(data) {
          }
         });
}

function on_add_round_to_queue(evt) {
  var li = $(this);
  var li_roundid = li.attr('data-roundid');
  $("#new-roster-div").toggleClass('hidden',
                                   li_roundid !== false &&
                                   typeof li_roundid != typeof undefined);
  show_create_roster_dialog(li.attr('data-classid'), li.attr('data-round'));
}

function show_create_roster_dialog(classid, roundno) {
  // roster.new action:
  // For a follow-on round (round >= 2): $_POST['roundid'] + $_POST['top'] + ['bucketed']
  // For a first aggregate (round = 1 and no roundid), classid for the aggregate class, plus top + bucketed
  var cl = g_classes[classid];

  $("#add-to-queue-modal .hidable").addClass('hidden');
  if (roundno == 1) {
    $("#add-to-queue-modal .group-buckets").removeClass('hidden');
  } else if (g_use_subgroups) {
    $("#add-to-queue-modal .subgroup-buckets").removeClass('hidden');
  } else {
    $("#add-to-queue-modal .no-buckets").removeClass('hidden');
  }

  show_modal("#add-to-queue-modal", function (event) {
    close_modal("#add-to-queue-modal");
    // For roundno >= 2, the previous round may not be defined, so roster
    // contruction will have to figure out the roundid only at the very last
    // moment.
    add_round_to_queue(classid, roundno,
                       {after_action: $("#after-action-sel").val(),
                        runs_per_lane: $("#schedule-reps").val(),
                        top: Number($("#new-roster-top").val()),
                        bucketed:
                          roundno == 1 ? $("#bucketed_groups").is(':checked')
                          : g_use_subgroups ? $("#bucketed_subgroups").is(':checked')
                          : false}
                      );
    return false;
  });
}

// The after_action value is one of:
//  - A sceneid (val > 0)
//  - Stop racing (val = 0) : sceneid_at_finish = 0, continue_racing = 0
//  - Start next round (val = -1): sceneid_at_finish = 0, continue_racing = 1
function add_round_to_queue(classid, round, roster_params) {
  $.ajax('action.php',
         {type: 'POST',
          data: {action: 'playlist.add',
                 classid: classid,
                 round: round,
                 top: roster_params.top || 0,
                 bucketed: roster_params.bucketed ? 1 : 0,
                 n_times_per_lane: roster_params.runs_per_lane,
                 continue_racing: roster_params.after_action == 0 ? 0 : 1,
                 sceneid_at_finish: roster_params.after_action,
                },
          success: function(data) {
            if (data.hasOwnProperty('queue-entry')) {
              var entry = data['queue-entry'];
              console.log(entry);
              g_queue.push(entry);
              append_playlist_entry_li(entry, g_all_scenes);
              maybe_change_playlist_message();
              // TODO Maybe a new round
              build_rounds(g_queue, g_classes);
            }
          }
         });
}

// entry is a playlist queue entry, and includes:
// queueid, classid, round, roundname, bucket_limit, bucketed, n_times_per_lane,
// sceneid_at_finish, continue_racing
function append_playlist_entry_li(entry, all_scenes) {
  var top_id = 'top_' + entry.queueid;
  var bucketed_id = 'bucketed_' + entry.queueid;
  var reps_id = 'reps_' + entry.queueid;
  var after_id = 'after_' + entry.queueid;

  var collapsible = $("<div class='collapsible'/>").css('display', 'none');
  var round_entry = find_round(entry.classid, entry.round);
  if (!round_entry) {
    // Show roster params only if round doesn't exist
    collapsible.append($("<label>Choose top:</label>").attr('for', top_id))
      .append($("<input type='number'/>")
              .attr('id', top_id)
              .attr('value', entry.bucket_limit)
              .on('click', function (evt) { evt.stopPropagation(); })
              .on('change', on_change_bucket_limit));

    var bucketed = $("<input type='checkbox' class='flipswitch'/>")
        .attr('id', bucketed_id)
        .attr('data-off-text', 'Overall')
        .prop('checked', entry.bucketed == 0 ? false : true)
        .on('change', on_change_bucketed);
    if (g_classes[entry.classid].constituents.length > 0) {
      // Aggregate class
      bucketed.attr('data-on-text', 'Each ' + g_group_label);
    } else if (g_use_subgroups) {
      bucketed.attr('data-on-text', 'Each ' + g_subgroup_label);
    } else {
      bucketed = false;
    }
    if (bucketed) {
      collapsible
        .append("<br/>")
        .append($("<label>From:</label>").attr('for', bucketed_id))
        .append(bucketed);
    }
    collapsible.append("<br/>");
  }

  var reps = $("<select/>").attr('id', reps_id)
      .append("<option>1</option>")
      .append("<option>2</option>")
      .append("<option>3</option>")
      .append("<option>4</option>")
      .append("<option>5</option>")
      .append("<option>6</option>")
      .val(entry.n_times_per_lane)
      .on('change', on_change_times_per_lane);
  collapsible
    .append($("<label>Runs per lane:</label>").attr('for', reps_id))
    .append(reps);

  var after_sel = $("<select/>")
      .attr('id', after_id);
  collapsible.append($("<label>After racing:</label>").attr('for', after_id))
    .append(after_sel);
  build_after_action_select(after_sel, all_scenes);

  if (entry.sceneid_at_finish > 0) {
    after_sel.val(entry.sceneid_at_finish);
  } else if (parseInt(entry.continue_racing)) {
    after_sel.val(-1);
  } else {  // stop
    after_sel.val(0);
  }
  after_sel.on('change', on_change_sceneid_at_finish);

  var li = $("<li class='queue'/>")
      .attr('data-queueid', entry.queueid)
      .attr('data-classid', entry.classid)
      .attr('data-round', entry.round)
      .append($("<div class='queued-round'/>")
              .toggleClass('finished', round_entry != null &&
                           round_entry.heats_scheduled > 0 &&
                           round_entry.heats_run == round_entry.heats_scheduled)
              .toggleClass('partial', round_entry != null &&
                           round_entry.heats_scheduled > 0 &&
                           round_entry.heats_run > 0 &&
                           round_entry.heats_run < round_entry.heats_scheduled)
              .toggleClass('current',
                           g_current_round.classid == entry.classid &&
                           g_current_round.round == entry.round)
              .append($("<p/>")
                      .text(entry.roundname)
                      .prepend("<img class='triangle' src='img/triangle_east.png'/>")
                      .append($("<input type='button' class='queue-remove' value=' x '/>")
                              .on('click', on_remove_playlist_entry))
                     )
              .append(collapsible))
      .append("<p class='gap'><span class='after-action'/></p>")
      .on('click', on_open_playlist_entry)
      .appendTo('#queue-ul');

  update_playlist_entry_format(after_sel);

  mobile_select(reps);
  mobile_select(after_sel);
  if (bucketed) {
    flipswitch(bucketed);
    bucketed.parent().on('click', function(event) { event.stopPropagation(); });
  }
}

function build_rounds(queue, classes) {
  $('#rounds-ul').empty();

  // Maps classid to { highest:, offer_additional: }
  //  offer_additional is true if there no rounds for the class not either
  //  already in the queue or already completed.
  var per_class = {};
  $.each(classes, function(classid, cl) {
    per_class[classid] = {highest: 0, offer_additional: true};
  });

  var highest_round = 0;  // Highest round overall
  $.each(queue, function(i, entry) {
    // entry = {classid, round}
    if (entry.round > highest_round) {
      highest_round = entry.round;
    }
    if (entry.round > per_class[entry.classid].highest) {
      per_class[entry.classid].highest = entry.round;
    }
  });

  // Go through existing rounds to identify highest round per class,
  // highest round overall
  $.each(g_all_rounds, function(classid, round_entries) {
    $.each(round_entries, function(i, round_entry) {
      round_entry.round = round_entry.round;
      round_entry.heats_scheduled = round_entry.heats_scheduled;
      if (round_entry.round > highest_round) {
        highest_round = round_entry.round;
      }
      if (round_entry.round > per_class[classid].highest) {
        per_class[classid].highest = round_entry.round;
      }
    });
  });

  // Starting with round 1's, present each round not already in the queue
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
        var round_entry = find_round(classid, r);
        if (round_entry && round_entry.heats_run < round_entry.heats_scheduled) {
          per_class[classid].offer_additional = false;
        }

        // Offer the round for adding to the playlist if either:
        // (1) it's a round that's already created, or
        // (2) all the rounds for the class are already in the playlist, or have completed
        if (round_entry || (per_class[classid].offer_additional && per_class[classid].highest == r - 1)) {
          if (first_in_round) {
            $("#rounds-ul").append("<div class='spacer'/>");
            first_in_round = false;
          }
          $("#rounds-ul").append(
            $("<li/>")
              .text(round_entry ? round_entry.roundname : (cl.class + ', Round ' + r))
              .attr('data-classid', classid)
              .attr('data-round', r)
              .attr('data-roundid', function() {
                if (round_entry) {
                  return round_entry.roundid;
                }
              })
              .toggleClass('finished',
                           round_entry != null && round_entry.heats_run > 0 &&
                           round_entry.heats_run >= round_entry.heats_scheduled)
              .toggleClass('partial', round_entry != null &&
                           round_entry.heats_scheduled > 0 &&
                           round_entry.heats_run > 0 &&
                           round_entry.heats_run < round_entry.heats_scheduled)
              .toggleClass('current', round_entry != null &&
                           g_current_round.classid == round_entry.classid &&
                           g_current_round.round == round_entry.round)
              .on('click', on_add_round_to_queue)
          );
        }
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
      // Clicking on a list item wants to open it (on_open_playlist_entry); if
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
