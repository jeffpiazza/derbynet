$(function() {
    $("body").append('<div id="modal_background"></div>');
    $("body").append('<div id="second_modal_background"></div>');
    $("body").append('<div id="third_modal_background"></div>');
});

function do_show_modal(modal_selector, background_selector, z_index, submit_handler) {
    var modal_background = $(background_selector);
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
        'z-index': z_index,
        'left' : 50 + '%',
        'margin-left': -(modal_width/2) + "px"
    });
    modal_div.fadeTo(200, 1);
}

function do_close_modal(modal_selector, background_selector) {
    $(background_selector).fadeOut(200);
    $(modal_selector).addClass("hidden");
    $(modal_selector).css({'display': 'none'});
}

function show_modal(modal_selector, submit_handler) {
    do_show_modal(modal_selector, "#modal_background", 11000, submit_handler);
}

function close_modal(modal_selector) {
    do_close_modal(modal_selector, "#modal_background");
}

// For a modal that's supposed to appear in front of another ("ordinary") modal.
function show_secondary_modal(modal_selector, submit_handler) {
    do_show_modal(modal_selector, "#second_modal_background", 13000, submit_handler);
}

function close_secondary_modal(modal_selector) {
    do_close_modal(modal_selector, "#second_modal_background");
}

function show_tertiary_modal(modal_selector, submit_handler) {
    do_show_modal(modal_selector, "#third_modal_background", 15000, submit_handler);
}

function close_tertiary_modal(modal_selector) {
    do_close_modal(modal_selector, "#third_modal_background");
}
