
// Returns:
//    "" if acceptable
//    "unacceptable-hover-filled"
//    "unacceptable-hover-wrong-class"
function award_assignment_hover_class(award_li, racer_div) {
  // racer_div has data-racerid
  // award_li has data-awardid, awardname, awardtypeid, classid, class, rankid, rank, racerid (0)
  if (award_li.attr('data-racerid') != 0) {
    return "unacceptable-hover-filled";
  }
  var classid = award_li.attr('data-classid');
  var rankid = award_li.attr('data-rankid');
  if ((classid != 0 && classid != racer_div.find('div.group_name').attr('data-classid')) ||
      (rankid != 0 && rankid != racer_div.find('div.subgroup_name').attr('data-rankid'))) {
    return "unacceptable-hover-wrong-class";
  }

  return "";
}

function maybe_assign_award_winner(award_li, racer_div) {
  if (award_assignment_hover_class(award_li, racer_div) == "") {
        $.ajax('action.php',
               {type: 'POST',
                data: {action: 'award.winner',
                       awardid: award_li.attr('data-awardid'),
                       racerid: racer_div.attr('data-racerid')},
                cache: false,
                headers: { "cache-control": "no-cache" },
                success: function(data) {
                  update_awards(data);
                }
               });
      }
}

function show_racer_awards_modal(judging_racer) {
  var racer_id = judging_racer.attr('data-racerid');

  $("#racer_awards_recipient_carno").text(judging_racer.find('.carno').text());
  $("#racer_awards_recipient_name").text(judging_racer.find('.racer_name').text());
  $("#racer_awards_racerid").val(judging_racer.attr('data-racerid'));
  $("#racer_awards_awardname").val(judging_racer.attr('data-adhoc'));

  $("#racer_awards").empty();
  $("#awards li").each(function() {
    if ($(this).attr('data-racerid') == racer_id) {
      // TODO Provide a control to remove award
      $('<li></li>').text($(this).attr("data-awardname")).appendTo($("#racer_awards"));
    }
  });

  show_modal("#racer_awards_modal", function(event) {
    close_racer_awards_modal();
    console.log("Sending data: " + $("#racer_awards_form").serialize());  // TODO
    $.ajax('action.php',
           {type: 'POST',
            data:  $("#racer_awards_form").serialize(),
            cache: false,
            headers: { "cache-control": "no-cache" },
            success: function(data) {
              console.log("Response received: " + data);
              update_awards(data);
            }
           });
    return false;
  });
}

function close_racer_awards_modal() {
  close_modal("#racer_awards_modal");
}


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

  $(".award_marker, .adhoc_marker").addClass('hidden');  // Hide all the award markers, unhide as needed below
  $(".judging_racer").removeAttr('data-adhoc')
                     .css('background-image', 'none');

  var adhoc_count = 0;
  for (var i = 0; i < awards.length; ++i) {
    if (awards[i].getAttribute("adhoc") != 0) {
      var racerid = awards[i].getAttribute("racerid");
      $(".judging_racer[data-racerid='" + racerid + "']").attr('data-adhoc', awards[i].getAttribute('awardname'));
      $(".judging_racer[data-racerid='" + racerid + "'] .adhoc_marker").removeClass('hidden');
      ++adhoc_count;
    }
  }

  var speed_awards = dataxml.getElementsByTagName('speed-award');
  for (var i = 0; i < speed_awards.length; ++i) {
    var racerid = speed_awards[i].getAttribute("racerid");
    console.log("Marking speed award for " + racerid);  // TODO
    $(".judging_racer[data-racerid='" + racerid + "']").css("background-image", "url('img/laurel.png')");
  }
  
  // Add more empty list items, as necessary
  if ($("#awards li").length < awards.length - adhoc_count) {
    while ($("#awards li").length < awards.length - adhoc_count) {
      $('<li></li>')
        .append('<p class="awardname"></p>' +
                '<p>' +
                    '<span class="awardtype"></span>' +
                '</p>' +
                '<p class="class-and-rank">' +
                    '<span class="rankname"></span>' +
                    '<span class="classname"></span> ' +
                    '<a class="recipient"></a>' +
                '</p>')
        .appendTo('#awards ul');
    }
    make_awards_draggable_and_droppable();
  }
  
  // Update the list items to match the data from the server.
  // NOTE This assumes that all the ad hoc awards are at the end of the list, so
  // the named awards and the #awards <li> elements have matching indices.
  $("#awards li").each(function(i) {
    if (i >= awards.length - adhoc_count) {
      $(this).remove();
    } else {
      if (awards[i].getAttribute("adhoc") == 0) {
        $(this).attr("data-awardid", awards[i].getAttribute("awardid"));
        if ($(this).attr("data-awardname") != awards[i].getAttribute("awardname")) {
          $(this).find('p.awardname').text(awards[i].getAttribute("awardname"));
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

        var racerid = awards[i].getAttribute("racerid");
        if (racerid != '' && racerid != '0') {
          $(".judging_racer[data-racerid='" + racerid + "'] .award_marker").removeClass('hidden');
        }
        if ($(this).attr("data-racerid") != racerid) {
          $(this).attr("data-racerid", racerid);
          $(this).find('.recipient')
            .empty()
            .text(awards[i].getAttribute("firstname") + " " +
                  awards[i].getAttribute("lastname"));
          if (racerid != 0) {
            $(this).find('.recipient')
              .prepend('<img src="img/cancel-12.png" onclick="handle_remove_recipient($(this));"/>');
            $(this).on('click', function() { handle_remove_recipient($(this)); });
          }
          // An award with a recipient can't be re-awarded, so constrain its draggability
          var offsets = $(this).offset();
          // These offsets depend a lot on the size of the helper, and the cursorAt values
          $(this).draggable("option", "containment", racerid == "" || racerid == "0" ? false :
                            [ offsets.left, offsets.top , offsets.left + 240, offsets.top + 30 ] );
        }
      }
    }
  });
}

function handle_remove_recipient(imgxjq) {
  $.ajax('action.php',
         {type: 'POST',
          data: {action: 'award.winner',
                 awardid: imgxjq.closest("[data-awardid]").attr("data-awardid"),
                 racerid: 0},
          cache: false,
          headers: { "cache-control": "no-cache" },
          success: function(data) {
            update_awards(data);
          }
         });
}

function make_awards_draggable_and_droppable() {
  // NOTE This gets called on empty award li's, before any attributes have been populated on them.

  // Awards without recipients can be dragged to racers
  $("#awards li").draggable({
    helper: function() {
      return $("<img src='img/award-ribbon-54x72.png'/>");
    },
    // This interim cursor is a large image...
    cursorAt: { left: 27, top: 27 },
    appendTo: 'body',
    opacity: 0.5,
    // Element should revert to its start position when dragging stops, unless
    // dropped successfully.
    revert: function(racer) {
      return racer == false || award_assignment_hover_class($(this), racer) != "";
    },
  });

  // Awards accept racers dropped on them
  $("#awards li").droppable({
    accept: '.judging_racer',
    hoverClass: 'award-recipient-hover',
    tolerance: 'pointer',
    over: function(event, ui) {
      var award = $(event.target);
      var racer = $(ui.draggable);
      award.addClass(award_assignment_hover_class(award, racer));
    },
    out: function(event, ui) {
      $(event.target).removeClass('unacceptable-hover-filled unacceptable-hover-wrong-class');
    },
    drop: function(event, ui) {
      $(event.target).removeClass('unacceptable-hover-filled unacceptable-hover-wrong-class');
      maybe_assign_award_winner($(event.target), $(ui.draggable));
    }});
}

function make_racers_draggable_and_droppable() {
  // Racers are draggable to awards
  $('#racers .judging_racer').draggable({
    helper: 'clone',
    appendTo: 'body',
    opacity: 0.5,
    // Element should revert to its start position when dragging stops, unless
    // dropped successfully.
    revert: function(award_li) {
      return award_li == false || award_assignment_hover_class(award_li, $(this)) != "";
    },
  });

  // Racers accept awards dropped on them
  $('#racers .judging_racer').droppable({
    accept: '#awards li',
    hoverClass: 'award-recipient-hover',
    tolerance: 'pointer',
    over: function(event, ui) {
      var award = $(ui.draggable);
      var racer = $(event.target);
      racer.addClass(award_assignment_hover_class(award, racer));
    },
    out: function(event, ui) {
      $(event.target).removeClass('unacceptable-hover-filled unacceptable-hover-wrong-class');
    },
    drop: function(event, ui) {
      // Here, event.target is the racer and ui.draggable is the award
      $(event.target).removeClass('unacceptable-hover-filled unacceptable-hover-wrong-class');
      maybe_assign_award_winner( $(ui.draggable), $(event.target));
    }});
}

$(function() {
  make_racers_draggable_and_droppable();
  // TODO We want to mark racers with ad-hoc awards, just not list them as awardable
  $.ajax('action.php',
         {type: 'GET',
          data: {query: 'award.list'},
          cache: false,
          headers: { "cache-control": "no-cache" },
          success: function(data) {
            update_awards(data);
          }
         });
});
