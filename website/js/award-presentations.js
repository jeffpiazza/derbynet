g_current_award_key = '';

g_action_url = 'action.php';

function poll_for_current_award() {
    $.ajax(g_action_url,
           {type: 'GET',
            data: {query: 'award-presentations' /*, // TODO
                   key: g_current_award_key */},
            success: function(data) {
                setTimeout(poll_for_current_award, 500 /* ms. */);
                process_current_award(data);
            },
            error: function() {
                setTimeout(poll_for_current_award, 500 /* ms. */);
            }
           });
}

$(function() { poll_for_current_award(); });

function parse_award(data) {
    var award_xml = data.getElementsByTagName('award')[0];
     if (!award_xml) {
        return false;
    }
    return {key: award_xml.getAttribute('key'),
            awardname: award_xml.getAttribute('awardname'),
            carnumber: award_xml.getAttribute('carnumber'),
            carname: award_xml.getAttribute('carname'),
            firstname: award_xml.getAttribute('firstname'),
            lastname: award_xml.getAttribute('lastname'),
            headphoto: award_xml.getAttribute('headphoto'),
            carphoto: award_xml.getAttribute('carphoto')};
}

function process_current_award(data) {
    var award = parse_award(data);
    if (!award) {
        console.log("Returning early because there's no current award");
        console.log(data);
        $("#firstname").text("--");
        $("#lastname").text("--");
        return;
    }

    if (award.key != g_current_award_key) {
        $(".reveal").hide();

        $("#awardname").text(award.awardname);
        $("#carnumber").text(award.carnumber);
        $("#carname").text(award.carname);
        $("#firstname").text(award.firstname);
        $("#lastname").text(award.lastname);

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
        if (award.headphoto && award.headphoto.length > 0) {
            $("#headphoto").append("<img src=\"" + award.headphoto + "\"/>");
            $("#headphoto img").css('max-height', maxPhotoHeight);
        }
        $("#carphoto").empty();
        $("#carphoto").css('width', $(window).width() / 2 - 10);
        if (award.carphoto && award.carphoto.length > 0) {
            $("#carphoto").append("<img src=\"" + award.carphoto + "\"/>");
            $("#carphoto img").css('max-height', maxPhotoHeight);
        }
        g_current_award_key = award.key;

        setTimeout(function () {
            $(".reveal").fadeIn(1000);
        }, 2000);
    }
}
