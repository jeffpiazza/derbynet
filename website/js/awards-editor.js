// TODO Ordering in the face of speed trophies?

function awardtypeid_to_awardtype(awardtypeid, dataxml) {
  var types = dataxml.getElementsByTagName('awardtype');
  for (var i = 0; i < types.length; ++i) {
    if (types[i].getAttribute("awardtypeid") == awardtypeid) {
      return types[i].getAttribute("awardtype");
    }
  }
  return "Can't resolve awardtypeid " + awardtypeid;
}

function classid_to_class(classid, dataxml) {
  if (classid == 0) return "";
  var types = dataxml.getElementsByTagName('class');
  for (var i = 0; i < types.length; ++i) {
    if (types[i].getAttribute("classid") == classid) {
      return types[i].getAttribute("name");
    }
  }
  return "Can't resolve classid " + classid;
}

function rankid_to_rank(rankid, dataxml) {
  if (rankid == 0) return "";
  var types = dataxml.getElementsByTagName('rank');
  for (var i = 0; i < types.length; ++i) {
    if (types[i].getAttribute("rankid") == rankid) {
      return types[i].getAttribute("name");
    }
  }
  return "Can't resolve rankid " + rankid;
}

function update_awards(dataxml) {
  var awards = dataxml.getElementsByTagName('award');

  if ($("#all_awards").find('li').length < awards.length) {
    while ($("#all_awards").find('li').length < awards.length) {
      $('<li></li>')
        .append('<a>' +
                '<span class="awardname"></span>' +
                '<p>' +
                    '<span class="awardtype"></span>' +
                '</p>' +
                '<p>' +
                    '<span class="rankname"></span>' +
                    '<span class="classname"></span> ' +
                    '<span class="recipient"></span>' +
                '</p>' +
                '</a>')
        .append('<a onclick="handle_edit_award($(this).closest(\'li\'));"></a>')
        .appendTo('#all_awards');
    }
    $("#all_awards").listview("refresh");
    makeAwardsDroppable();
  }
  
  $("#all_awards").find('li').each(function(i) {
    if (i >= awards.length) {
      $(this).remove();
    } else {
      $(this).attr("data-awardid", awards[i].getAttribute("awardid"));
      if ($(this).attr("data-awardname") != awards[i].getAttribute("awardname")) {
        $(this).find('a').find('span.awardname').text(awards[i].getAttribute("awardname"));
        $(this).attr("data-awardname", awards[i].getAttribute("awardname"));
      }
      if ($(this).attr("data-awardtypeid") != awards[i].getAttribute("awardtypeid")) {
        $(this).attr("data-awardtypeid", awards[i].getAttribute("awardtypeid"));
        $(this).find('.awardtype').text(
          awardtypeid_to_awardtype(awards[i].getAttribute("awardtypeid"), dataxml));
      }

      if ($(this).attr("data-classid") != awards[i].getAttribute("classid")) {
        $(this).attr("data-classid", awards[i].getAttribute("classid"));
        var classname = classid_to_class(awards[i].getAttribute("classid"), dataxml);
        $(this).attr("data-class", classname);
        $(this).find('.classname').text(classname);
      }
      
      if ($(this).attr("data-rankid") != awards[i].getAttribute("rankid")) {
        var rankid = awards[i].getAttribute("rankid");
        $(this).attr("data-rankid", rankid);
        var rankname = rankid_to_rank(rankid, dataxml);
        $(this).attr("data-rank", rankname);
        if (rankid != 0) {
          $(this).find('.rankname').text(rankname + ', ');
        }
      }

      if ($(this).attr("data-racerid") != awards[i].getAttribute("racerid")) {
        $(this).attr("data-racerid", awards[i].getAttribute("racerid"));
        $(this).find('.recipient')
          .empty()
          .text(awards[i].getAttribute("firstname") + " " +
                awards[i].getAttribute("lastname"));
        if (awards[i].getAttribute("racerid") != 0) {
          $(this).find('.recipient')
            .prepend('<img src="img/cancel-12.png" onclick="handle_remove_recipient($(this));"/>')
        }
      }
    }
  });
}

function handle_remove_recipient(imgxjq) {
  $.ajax(g_action_url,
         {type: 'POST',
          data: {action: 'award.edit',
                 awardid: imgxjq.closest("[data-awardid]").attr("data-awardid"),
                 racerid: 0},
          success: function(data) {
            update_awards(data);
          }
         });
}

function handle_new_award() {
  
  $('#award_editor_form').find('input[name="awardid"]').val('new');
  $('#award_editor_form').find('input[name="name"]').val('');
  // TODO $('#award_editor_form').find('select[name="awardtypeid"]').val(awardtypeid).change();
  // TODO $('#award_editor_form').find('select[name="class_and_rank"]').val('0,0').change();

  show_modal('#award_editor_modal', function() {
    close_modal('#award_editor_modal');
    $.ajax(g_action_url,
           {type: 'POST',
            data: $('#award_editor_form').serialize(),
            success: function(data) {
              update_awards(data);
            }
           });
    return false;
  });
}

function handle_edit_award(list_item) {
  var awardid = list_item.attr('data-awardid');
  var awardname = list_item.attr('data-awardname');
  var awardtypeid = list_item.attr('data-awardtypeid');
  var rankid = list_item.attr('data-rankid');
  var classid = list_item.attr('data-classid');
  
  $('#award_editor_form').find('input[name="awardid"]').val(awardid);
  $('#award_editor_form').find('input[name="name"]').val(awardname);
  $('#award_editor_form').find('select[name="awardtypeid"]').val(awardtypeid).change();
  $('#award_editor_form').find('select[name="class_and_rank"]').val(classid + ',' + rankid).change();

  show_modal('#award_editor_modal', function() {
    close_modal('#award_editor_modal');
    $.ajax(g_action_url,
           {type: 'POST',
            data: $('#award_editor_form').serialize(),
            success: function(data) {
              update_awards(data);
            }
           });
    return false;
  });
}

// Assumes that we were invoked through the award_editor_modal, and that
// therefore the relevant award information is available in the form.
function handle_delete_award(formjq) {
  close_modal('#award_editor_modal');
  if (confirm('Really delete award "' + $('#award_editor_form input[name="name"]').val() + '"?')) {
    $.ajax(g_action_url,
           {type: 'POST',
            data: {action: 'award.delete',
                   awardid: $('#award_editor_form input[name="awardid"]').val()},
            success: function(data) {
              update_awards(data);
            }
           });
  }
  return false;
}

// Returns 0 if acceptable,
// 1 if already filled
// 2 if unacceptable because of classid/rankid
function acceptableAwardAssignment(awardjq, racerjq) {
  var racerid = awardjq.attr("data-racerid");
  if (racerid != 0) {
    return 1;
  }
  var classid = awardjq.attr("data-classid");
  if (classid != 0 && racerjq.attr("data-classid") != classid) {
    return 2;
  }
  var rankid = awardjq.attr("data-rankid");
  if (rankid != 0 && racerjq.attr("data-rankid") != rankid) {
    return 2;
  }
  return 0;
}

// A racer can be dragged onto an award to "claim" the award for that racer.
function makeAwardsDroppable() {
  $("#all_awards li").droppable({
    hoverClass: "award_recipient_hover",
    over: function(event, ui) {
      var draggable = $(ui.draggable);
      var target = $(event.target);
      // This event also arises when re-ordering the awards themselves
      if (draggable[0].hasAttribute("data-racerid") &&
          !draggable[0].hasAttribute("data-awardid")) {
        var acceptable = acceptableAwardAssignment(target, draggable);
        target.toggleClass('unacceptable_hover', acceptable != 0);
        target.toggleClass('unacceptable_class_hover', acceptable == 2);
      }
    },
    out: function(event, ui) {
      $(event.target).removeClass('unacceptable_hover unacceptable_class_hover');
    },
    drop: function(event, ui) {
      $(event.target).removeClass('unacceptable_hover unacceptable_class_hover');
      if (acceptableAwardAssignment($(event.target), $(ui.draggable)) == 0) {
        $.ajax(g_action_url,
               {type: 'POST',
                data: {action: 'award.edit',
                       awardid: $(event.target).attr('data-awardid'),
                       racerid: $(ui.draggable).attr('data-racerid')},
                success: function(data) {
                  update_awards(data);
                }
               });
      }
    }});
}

$(function() {
  $("#all_awards").sortable({stop: function(event, ui) {
    var data = {action: 'award.order'};
    $("#all_awards li").each(function (i) {
      data['awardid_' + (i + 1)] = $(this).attr('data-awardid');
    });

    $.ajax(g_action_url,
           {type: 'POST',
            data: data,
            success: function(data) {
              update_awards(data);
            }
           });
  }});

  // Make the racer list items draggable (to an award)
  $('#racers li').draggable({
    helper: 'clone',
    appendTo: 'body',
    opacity: 0.5,
    revert: 'invalid'
  });

  // TODO Poll on this, not just this one time
  $.ajax(g_action_url,
         {type: 'GET',
          data: {query: 'award.list'},
          success: function(data) {
            update_awards(data);
          }
         });
});
