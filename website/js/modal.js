$(function() {
    $("body").append('<div id="modal_background"></div>');
    $("body").append('<div id="second_modal_background"></div>');
});

function show_modal(modal_selector, submit_handler) {
    var modal_background = $("#modal_background");
    modal_background.css({'display': 'block',
                          'opacity': 0});
    modal_background.fadeTo(200, 0.5);

    var modal_div = $(modal_selector);
    if (typeof submit_handler != 'undefined' && submit_handler) {
        var form = modal_div.find("form");
        form.off("submit");
        form.on("submit", submit_handler);
    }

    var modal_width = modal_div.outerWidth();
    modal_div.removeClass("hidden");
    modal_div.css({ 
        'display': 'block',
        'position': 'fixed',
        'opacity': 0,
        'z-index': 11000,
        'left' : 50 + '%',
        'margin-left': -(modal_width/2) + "px"
    });
    modal_div.fadeTo(200, 1);
}

function close_modal(modal_selector) {
    $("#modal_background").fadeOut(200);
    $(modal_selector).addClass("hidden");
    $(modal_selector).css({'display': 'none'});
}

// For a modal that's supposed to appear in front of another ("ordinary") modal.
function show_secondary_modal(modal_selector, submit_handler) {
    var modal_background = $("#second_modal_background");
    modal_background.css({'display': 'block',
                          'opacity': 0});
    modal_background.fadeTo(200, 0.5);

    var modal_div = $(modal_selector);
    if (typeof submit_handler != 'undefined' && submit_handler) {
        var form = modal_div.find("form");
        form.off("submit");
        form.on("submit", submit_handler);
    }

    var modal_width = modal_div.outerWidth();
    modal_div.removeClass("hidden");
    modal_div.css({ 
        'display': 'block',
        'position': 'fixed',
        'opacity': 0,
        'z-index': 13000,
        'left' : 50 + '%',
        'margin-left': -(modal_width/2) + "px"
    });
    modal_div.fadeTo(200, 1);
}

function close_secondary_modal(modal_selector) {
    $("#second_modal_background").fadeOut(200);
    $(modal_selector).addClass("hidden");
    $(modal_selector).css({'display': 'none'});
}