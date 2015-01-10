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
        $("#firstname").text(award.firstname);
        $("#lastname").text(award.lastname);
        $("#headphoto").empty();
        if (award.headphoto) {
            $("#headphoto").append("<img src=\"" + award.headphoto + "\"/>");
        }
        $("#carphoto").empty();
        if (award.carphoto) {
            $("#carphoto").append("<img src=\"" + award.carphoto + "\"/>");
        }
        g_current_award_key = award.key;

        setTimeout(function () {
            $(".reveal").fadeIn(1000);
        }, 2000);
    }
}
