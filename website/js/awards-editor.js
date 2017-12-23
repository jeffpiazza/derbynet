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

  if ($("#all_awards li").length < awards.length) {
    while ($("#all_awards li").length < awards.length) {
      $('<li></li>')
        .append('<a>' +
                '<span class="awardname"></span>' +
                '<p>' +
                    '<span class="awardtype"></span>' +
                '</p>' +
                '<p class="class-and-rank">' +
                    '<span class="rankname"></span>' +
                    '<span class="classname"></span> ' +
                    '<span class="recipient"></span>' +
                '</p>' +
               '</a>')
        .append('<a onclick="handle_edit_award($(this).closest(\'li\'));"></a>')
        .appendTo('#all_awards');
    }
    $("#all_awards").listview("refresh");
  }
  
  $("#all_awards li").each(function(i) {
    if (i >= awards.length) {
      $(this).remove();
    } else {
      $(this).attr("data-awardid", awards[i].getAttribute("awardid"));
      if ($(this).attr("data-awardname") != awards[i].getAttribute("awardname")) {
        $(this).find('span.awardname').text(awards[i].getAttribute("awardname"));
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
      }
    }
  });
}

function handle_new_award() {
  $('#award_editor_form').find('input[name="awardid"]').val('new');
  $('#award_editor_form').find('input[name="name"]').val('');
  // TODO $('#award_editor_form').find('select[name="awardtypeid"]').val(awardtypeid).change();
  // TODO $('#award_editor_form').find('select[name="class_and_rank"]').val('0,0').change();
  $('.delete_button').addClass('hidden');

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
  $('.delete_button').removeClass('hidden');

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

function handle_delete_award() {
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

  // TODO Poll on this, not just this one time
  $.ajax(g_action_url,
         {type: 'GET',
          data: {query: 'award.list',
                 adhoc: '0'},
          success: function(data) {
            update_awards(data);
          }
         });
});
