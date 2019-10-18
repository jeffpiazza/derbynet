// Whether or not to show confetti is a kiosk parameter.
var g_confetti = false;
$(function() {
  KioskPoller.param_callback = function(parameters) {
    g_confetti = parameters.confetti;
  };
});

var AwardPoller = {
  current_award_key: null,
  // revealed tells whether the previous poll result said to reveal the
  // recipient -- we trigger fade-in and video on the transition from
  // not-revealed to revealed.
  revealed: true,

  query_for_current_award: function() {
    $.ajax('action.php',
           {type: 'GET',
            data: {query: 'award.current'},
            success: function(data) {
              AwardPoller.queue_next_query();
              AwardPoller.process_current_award(data);
            },
            error: function() {
              AwardPoller.queue_next_query();
            }
           });
  },

  queue_next_query: function() {
    setTimeout(function() { AwardPoller.query_for_current_award(); }, 500 /* ms. */);
  },

  parse_award: function(data) {
    var award_xml = data.getElementsByTagName('award')[0];
    if (!award_xml) {
      return false;
    }
    return {key: award_xml.getAttribute('key'),
            reveal: award_xml.getAttribute('reveal') == 'true',
            awardname: award_xml.getAttribute('awardname'),
            carnumber: award_xml.getAttribute('carnumber'),
            carname: award_xml.getAttribute('carname'),
            firstname: award_xml.getAttribute('firstname'),
            lastname: award_xml.getAttribute('lastname'),
            subgroup: award_xml.getAttribute('subgroup'),
            headphoto: award_xml.getAttribute('headphoto'),
            carphoto: award_xml.getAttribute('carphoto')};
  },

  process_current_award: function(data) {
    var award = this.parse_award(data);
    if (!award) {
      $("#awardname").text("Award Presentation");
      $(".reveal").hide();
      return;
    }

    if (this.current_award_key != award.key) {
      $(".reveal").hide();

      $("#awardname").text(award.awardname);
      $("#carnumber").text(award.carnumber);
      $("#firstname").text(award.firstname);
      $("#lastname").text(award.lastname);
      if (award.carname && award.carname.length > 0) {
        $("#carname").text(award.carname);
        $("#carname").css('display', 'block');
      } else {
        $("#carname").css('display', 'none');
      }
      if (award.subgroup && award.subgroup.length > 0) {
        $("#subgroup").text(award.subgroup);
        $("#subgroup").css('display', 'block');
      } else {
        $("#subgroup").css('display', 'none');
      }
      // Need to account for the height of the award-racer text, even though
      // it's presently hidden.
      var previousCss  = $("#award-racer-text").attr("style");
      $("#award-racer-text")
        .css({
          position:   'absolute',
          visibility: 'hidden',
          display:    'block'
        });
      var textHeight = $("#award-racer-text").height();
      $("#award-racer-text").attr("style", previousCss ? previousCss : "");

      // TODO Literal 10 vaguely accounts for margins, but is basically just a guess.
      var maxPhotoHeight = $(window).height() - ($("#photos").offset().top + textHeight) - 10;

      $("#headphoto").empty();
      $("#headphoto").css('width', $(window).width() / 2 - 10);
      $("#headphoto").css('margin-left', 0);

      $("#carphoto").empty();
      $("#carphoto").css('width', $(window).width() / 2 - 10);
      $("#carphoto").css('margin-right', 0);

      if (award.headphoto && award.headphoto.length > 0) {
        $("#headphoto").append("<img src=\"" + award.headphoto + "\"/>");
        $("#headphoto img").css('max-height', maxPhotoHeight);
      } else {
        // If there's no head photo, center the car photo by adjusting its margin
        $("#carphoto").css('margin-right', $(window).width() / 4);
      }
      if (award.carphoto && award.carphoto.length > 0) {
        $("#carphoto").append("<img src=\"" + award.carphoto + "\"/>");
        $("#carphoto img").css('max-height', maxPhotoHeight);
      } else {
        // If there's no car photo, center the head photo
        $("#headphoto").css('margin-left', $(window).width() / 4);
      }
      // Changing awards: prime revealed=false so next reveal (or current one)
      // will trigger fade-in and video.
      this.revealed = false;
    }

    if (!award.reveal) {
      $(".reveal").hide();

      var video = $("video.confetti");
      video.hide();
      video.get(0).pause();
      video.get(0).currentTime = 0;
      video.show();
    } else if (!this.revealed) {
      $(".reveal").fadeIn(1000);
      if (g_confetti && this.current_award_key != null) {
        setTimeout(function() {
          $("video.confetti").get(0).play(); }, 500);
        // Confetti is a 10-second video, and <video> element says loop="true";
        // fade out after 20 seconds to play video twice.
        setTimeout(function() {
          $("video.confetti").fadeOut(
            500, function() {
              var video = $("video.confetti");
              video.get(0).pause();
              video.get(0).currentTime = 0;
              video.show();
            }); },
                   20500);
      }
    }
    this.revealed = award.reveal;
    this.current_award_key = award.key;
  }
}

$(function() { AwardPoller.queue_next_query(); });

