// Polyfill for Array.indexOf, from MDN.  Covers a lot of cases we don't care about.
if (!Array.prototype.indexOf)
  Array.prototype.indexOf =
  (function(Object, max, min) {
    "use strict"
    return function indexOf(member, fromIndex) {
      if (this === null || this === undefined)
        throw TypeError("Array.prototype.indexOf called on null or undefined")

      var that = Object(this), Len = that.length >>> 0, i = min(fromIndex | 0, Len)
      if (i < 0) i = max(0, Len + i)
      else if (i >= Len) return -1

      if (member === void 0) {        // undefined
        for (; i !== Len; ++i) if (that[i] === void 0 && i in that) return i
      } else if (member !== member) { // NaN
        return -1 // Since NaN !== NaN, it will never be found. Fast-path it.
      } else                          // all else
        for (; i !== Len; ++i) if (that[i] === member) return i 

      return -1 // if the value was not found, then return -1
    }
  })(Object, Math.max, Math.min)

function set_up_ballot() {
  $("#awards div[data-awardid]").addClass("hidden");
  for (var awardid in g_ballot) {
    var award_ballot = g_ballot[awardid];
    $("div.please-click").removeClass('hidden');
    var div = $("#awards div[data-awardid=" + awardid + "]");
    div.removeClass("hidden");
    div.find(".please-vote-count").text(award_ballot['max_votes']);

    div.find("div.selection").remove();

    var award_ballot_votes = award_ballot['votes'];
    for (var i = 0; i < award_ballot_votes.length; ++i) {
      var img = $("<img></img>")
          .attr('src', thumbnail_url_for_racerid(award_ballot_votes[i]));
      $("<div class='selection'></div>")
        .append("<div class='carno'>" + car_number_for_racerid(award_ballot_votes[i]) + "</div>")
        .append(img)
        .appendTo(div);
    }
  }
  $("#no-awards").toggleClass('hidden', $("div.award").not('.hidden').length != 0);
}

// From the main screen, clicking on an award opens the "racers" modal, which
// lets the user choose a racer.
function click_one_award(div) {
  g_awardid = div.attr('data-awardid');
  write_racers_headline();
  set_full_ballot_message();

  var award_name = div.find(".award-name").text();

  var ww = $(window).width();
  var wh = $(window).height();
  $("#racers_modal").width(ww - 200).height(wh - 200);
  $("#racer_view_award_name").text(award_name);

  $("#racers div.ballot_racer").removeClass('hidden');

  var classids = div.attr('data-eligible-classids').split(',');
  var rankids = div.attr('data-eligible-rankids').split(',');
  $("#racers div.ballot_racer").each(function() {
    $(this).toggleClass('hidden',
                        classids.indexOf($(this).attr('data-classid')) < 0 ||
                        rankids.indexOf($(this).attr('data-rankid')) < 0);
  });
  
  show_modal("#racers_modal", function() {});
}

function write_racers_headline() {
  $("#selected_racers").empty();

  var award_ballot_votes = g_ballot[g_awardid]['votes'];
  for (var i = 0; i < award_ballot_votes.length; ++i) {
    var img = $("<img></img>")
        .attr('src', thumbnail_url_for_racerid(award_ballot_votes[i]));

    $("<div class='selection' data-racerid='" + award_ballot_votes[i] + "' onclick='show_racer_view_modal($(this));'></div>")
      .append("<div class='carno'>"
              + car_number_for_racerid(award_ballot_votes[i]) + "</div>")
      .append(img)
      .appendTo($("#selected_racers"));
  }
}

// Close-up view of a single racer, along with the checkbox to vote this racer
// in or out.
// div needs a data-racerid attribute
function show_racer_view_modal(div) {
  // g_awardid gets populated by click_one_award
  g_racerid = parseInt($(div).attr('data-racerid'));
  set_full_ballot_message();
  
  set_checkbox(g_ballot[g_awardid]['votes'].includes(g_racerid));
  $("#racer_view_carnumber").text(car_number_for_racerid(g_racerid));
  $("#racer_view_photo").attr('src', photo_url_for_racerid(g_racerid));
  show_secondary_modal("#racer_view_modal", function() {});
}

function toggle_vote(div) {
  var award_ballot = g_ballot[g_awardid];
  if (award_ballot['votes'].includes(g_racerid)) {
    award_ballot['votes'] =
      award_ballot['votes'].filter(function(v) { return v != g_racerid; });
    div.find('img').attr('src', 'img/checkbox-without-check.png');
  } else if (award_ballot['votes'].length >= award_ballot['max_votes']) {
    console.log("Full ballot!");
  } else {
    award_ballot['votes'].push(g_racerid);
    div.find('img').attr('src', 'img/checkbox-with-check.png');
  }

  $.ajax('action.php',
         {type: 'POST',
          data: {action: 'vote.cast',
                 awardid: g_awardid,
                 'votes': JSON.stringify(award_ballot['votes'])}
         });

  write_racers_headline();
  set_up_ballot();
}
    
function set_checkbox(checked) {
  var div = $("#racer_view_check");
  div.find('img').attr('src',
                       checked ? 'img/checkbox-with-check.png' :  'img/checkbox-without-check.png');
}

function set_full_ballot_message() {
  var award_ballot = g_ballot[g_awardid];
  var max_votes = award_ballot['max_votes'];

  $("#racer_view_max_votes").text(max_votes);

  $("#full-ballot-max").text(max_votes);

  console.log("Award_ballot:");console.log(award_ballot);
  console.log("g_racerid: " + g_racerid + ", includes=" + (award_ballot['votes'].includes(g_racerid)));
  console.log("max_votes=" + max_votes + ", votes length=" + award_ballot['votes'].length);
  $("#full-ballot").toggleClass('hidden',
                                award_ballot['votes'].includes(g_racerid) ||
                                award_ballot['votes'].length < max_votes);
}

function thumbnail_url_for_racerid(racerid) {
  // Depends on there already being thumbnail images available in the
  // div.ballot_racer blocks.
  return $("div.ballot_racer[data-racerid=" + racerid + "] img").attr('src');
}

function photo_url_for_racerid(racerid) {
  // Depends on there already being thumbnail images available in the
  // div.ballot_racer blocks.
  return $("div.ballot_racer[data-racerid=" + racerid + "]").attr('data-img');
}

function car_number_for_racerid(racerid) {
  return $("div.ballot_racer[data-racerid=" + racerid + "] div.carno").text();
}

// If balloting is open, there'll be an initial call to get_ballot, including
// the current value of the password field, even if it hasn't been displayed to
// the user yet..  If a password is needed, ballot.get will return a failure for
// password, which prompts the display of the password form.
function get_ballot() {
  $.ajax('action.php',
         {type: 'GET',
          data: {query: 'ballot.get',
                 password: $("#password_input").val()},
          success: function(data) {
            var password_modal_showing =
                $("#password_modal").closest('.modal_frame').length > 0 &&
                !$("#password_modal").closest('.modal_frame').is(".hidden");
            if (data.hasOwnProperty('outcome') && data.outcome.summary == 'failure') {
              $("#awards div[data-awardid]").addClass('hidden');
              if (data.outcome.code == 'password') {
                // If we're already showing the password modal, then show the
                // message about this having been the wrong password.  If we
                // weren't showing the password modal, then a "wrong password"
                // message is inappropriate, so hidden.
                $("#wrong-password").toggleClass('hidden', !password_modal_showing);
                if (!password_modal_showing) {
                  show_modal("#password_modal", function() { get_ballot(); return false; });
                }
              }
            } else {
              g_ballot = data.ballot;
              close_modal("#password_modal");
              set_up_ballot();
            }
          }
         });
}


$(function() {
  var balloting_open_or_closed = '';
  setInterval(function() {
    $.ajax('action.php',
           {type: 'GET',
            data: {query: 'settings.list',
                   key: 'balloting'},
            success: function(data) {
              var v = data.settings?.balloting || 'closed'
              if (balloting_open_or_closed == '') {
                balloting_open_or_closed = v;
              } else if (balloting_open_or_closed != v) {
                location.reload(true);
              }
            }
           });
  }, 5000);
});
