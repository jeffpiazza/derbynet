
// Returns:
//    "" if acceptable
//    "unacceptable-hover-filled"
//    "unacceptable-hover-wrong-class"
function award_assignment_hover_class(award_li, racer_div) {
  // racer_div has data-racerid
  // award_li has data-awardid, data-awardname, data-awardtypeid,
  //    data-classid, data-class, data-rankid, data-rank,
  //    data-eligible-classids, data-eligible-rankids, data-racerid (0)
  if (award_li.attr('data-racerid') != 0) {
    return "unacceptable-hover-filled";
  }
  var eligible_classids = award_li.attr('data-eligible-classids');
  if (typeof eligible_classids == typeof undefined || eligible_classids === false) {
    eligible_classids = '';
  }
  var eligible_rankids = award_li.attr('attr-eligible-rankids');
  if (typeof eligible_rankids == typeof undefined || eligible_rankids === false) {
    eligible_rankids = '';
  }
  if ((eligible_classids != '' &&
       !eligible_classids.split(',').includes(racer_div.find('div.group_name').attr('data-classid'))) ||
      (eligible_rankids != '' &&
       !eligible_rankids.split(',').includes(racer_div.find('div.group_name').attr('data-rankid')))) {
    return "unacceptable-hover-wrong-class";
  }

  return "";
}

function maybe_assign_award_winner(award_li, racer_div) {
  if (award_assignment_hover_class(award_li, racer_div) == "") {
        $.ajax('action.php',
               {type: 'POST',
                data: {action: 'json.award.winner',
                       awardid: award_li.attr('data-awardid'),
                       racerid: racer_div.attr('data-racerid')},
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
    $.ajax('action.php',
           {type: 'POST',
            // action=json.award.adhoc
            data:  $("#racer_awards_form").serialize(),
            success: function(data) {
              update_awards(data);
            }
           });
    return false;
  });
}

function close_racer_awards_modal() {
  close_modal("#racer_awards_modal");
}

function on_vote_choose(event) {
  $.ajax('action.php',
         {type: 'POST',
          data: {action: 'json.award.winner',
                 awardid: $(event.target).attr('data-awardid'),
                 racerid: $(event.target).attr('data-racerid')},
          success: function(data) {
            update_awards(data);
          }
         });
  close_modal("#ballot_results_modal");
}

function on_votes_click(event) {
  $("#ballot_results_tabulation").empty();
  var votes = $(event.target).data('votes');
  var awardid = $(event.target).attr('data-awardid');
  for (var i = 0; i < votes.length; ++i) {
    $("#ballot_results_tabulation").append(
      $("<div/>")
        .text(votes[i].score + " vote(s) for "
              + votes[i].carnumber + ": " + votes[i].firstname + " " + votes[i].lastname)
        .append($("<input type='button' value='Choose'/>")
                .attr('data-awardid', awardid)
                .attr('data-racerid', votes[i].racerid)
                .on('click', on_vote_choose))
    );
  }

  show_modal("#ballot_results_modal");
  
  return false;
}


function classid_to_class(classid, classes) {
  if (classid == 0) return "";
  for (var i = 0; i < classes.length; ++i) {
    if (classes[i].classid == classid) {
      return classes[i].name;
    }
  }
  return "Can't resolve classid " + classid;
}

function rankid_to_rank(rankid, classes) {
  if (rankid == 0) return "";
  for (var i = 0; i < classes.length; ++i) {
    for (var j = 0; j < classes[i].ranks.length; ++j) {
      if (classes[i].ranks[j].rankid == rankid) {
        return classes[i].ranks[j].name;
      }
    }
  }
  return "Can't resolve rankid " + rankid;
}

function update_awards(data) {
  var awards = data.awards;

  $(".award_marker, .adhoc_marker").addClass('hidden');  // Hide all the award markers, unhide as needed below
  $(".judging_racer").removeAttr('data-adhoc')
                     .css('background-image', 'none');

  var adhoc_count = 0;
  for (var i = 0; i < awards.length; ++i) {
    if (awards[i].adhoc) {
      var racerid = awards[i]["racerid"];
      $(".judging_racer[data-racerid='" + racerid + "']")
        .attr('data-adhoc', awards[i].awardname);
      $(".judging_racer[data-racerid='" + racerid + "'] .adhoc_marker").removeClass('hidden');
      ++adhoc_count;
    }
  }

  var speed_awards = data["speed-awards"];
  for (var i = 0; i < speed_awards.length; ++i) {
    var racerid = speed_awards[i].racerid;
    $(".judging_racer[data-racerid='" + racerid + "']")
      .css("background-image", "url('img/laurel.png')");
  }
  
  // Add more empty list items, as necessary
  if ($("#awards li").length < awards.length - adhoc_count) {
    while ($("#awards li").length < awards.length - adhoc_count) {
      $('<li></li>')
        .append($('<a class="votes">Votes</a>').on('click', on_votes_click))
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
      if (!awards[i].adhoc) {
        $(this).attr("data-awardid", awards[i].awardid);
        if ($(this).attr("data-awardname") != awards[i].awardname) {
          $(this).find('p.awardname').text(awards[i].awardname);
          $(this).attr("data-awardname", awards[i].awardname);
        }
        if ($(this).attr("data-awardtypeid") != awards[i].awardtypeid) {
          $(this).attr("data-awardtypeid", awards[i].awardtypeid);
          $(this).find('.awardtype').text(awards[i].awardtype);
        }

        if ($(this).attr("data-classid") != awards[i].classid) {
          $(this).attr("data-classid", awards[i].classid);
          var classname = classid_to_class(awards[i].classid, data.classes);
          $(this).attr("data-class", classname);
          $(this).find('.classname').text(classname);
        }

        var eligible_classids = awards[i]['eligible-classids'].join(',');
        if ($(this).attr("data-eligible-classids") != eligible_classids) {
          $(this).attr("data-eligible-classids", eligible_classids);
        }
        var eligible_rankids = awards[i]['eligible-rankids'].join(',');
        if ($(this).attr("data-eligible-rankids") != eligible_rankids) {
          $(this).attr("data-eligible-rankids", eligible_rankids);
        }

        if ($(this).attr("data-rankid") != awards[i].rankid) {
          var rankid = awards[i].rankid;
          $(this).attr("data-rankid", rankid);
          var rankname = rankid_to_rank(rankid, data.classes);
          $(this).attr("data-rank", rankname);
          if (rankid != 0) {
            $(this).find('.rankname').text(rankname + ', ');
          }
        }

        $(this).find('.votes')
          .data('votes', awards[i].votes.join(','))
          .attr('data-awardid', awards[i].awardid)
          .toggleClass('hidden', awards[i].votes.length == 0);
        
        var racerid = awards[i].racerid;
        if (racerid != '' && racerid != '0') {
          $(".judging_racer[data-racerid='" + racerid + "'] .award_marker").removeClass('hidden');
        }
        if ($(this).attr("data-racerid") != racerid) {
          $(this).attr("data-racerid", racerid);
          $(this).find('.recipient')
            .empty()
            .text(awards[i].firstname + " " + awards[i].lastname);
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

  $("#awards-empty").toggleClass('hidden', $("#awards li").length != 0);
  update_ballot_awards(awards);
}

function on_ballot_depth_change(event) {
  var awardid = $(event.target).attr('data-awardid');
  $.ajax('action.php',
         {type: 'POST',
          data: {action: 'json.award.edit',
                 awardid: awardid,
                 ballot_depth: $(event.target).val()},
          success: function(data) {
            update_awards(data);
          }
         });
}

function update_ballot_awards(awards) {
  $("#ballot_modal_awards").empty();
  for (var i = 0; i < awards.length; ++i) {
    var id = 'ballot_' + awards[i].awardid;
    $("#ballot_modal_awards").append(
      $("<div>")
        .append($("<h3/>").text(awards[i].awardname))
        .append($("<label/>").attr('for', id).text("Votes allowed per ballot:"))
        .append($("<select/>")
                .attr('id', id)
                .attr('data-awardid', awards[i].awardid)
                .append("<option value='0'>Not on ballot</option>")
                .append("<option>1</option>")
                .append("<option>2</option>")
                .append("<option>3</option>")
                .append("<option>4</option>")
                .append("<option>5</option>")
                .append("<option>6</option>")
                .val(awards[i]['ballot_depth'])
                .on('change', on_ballot_depth_change)));
  }
}

function handle_remove_recipient(imgxjq) {
  $.ajax('action.php',
         {type: 'POST',
          data: {action: 'json.award.winner',
                 awardid: imgxjq.closest("[data-awardid]").attr("data-awardid"),
                 racerid: 0},
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

function query_for_award_list() {
  $.ajax('action.php',
         {type: 'GET',
          data: {query: 'json.award.list'},
          success: function(data) {
            update_awards(data);
          }
         });
}

$(function() {
  make_racers_draggable_and_droppable();

  $("#balloting_state").on('change', function() {
    $.ajax('action.php',
           {type: 'POST',
            data: {action: 'json.ballot.open',
                   state: $("#balloting_state").is(':checked') ? "open" : "closed" }
           });
  });

  $("#ballot_password").on('keyup mouseup', function() {
    $.ajax('action.php',
           {type: 'POST',
            data: {action: 'settings.write',
                   ballot_password: $("#ballot_password").val()}
           });
  });
  
  
  // TODO We want to mark racers with ad-hoc awards, just not list them as awardable
  query_for_award_list();
  setInterval(function() { query_for_award_list(); }, 30000);
});
