$(function() {
  $("body").append('<div id="modal_background"></div>');
  $("body").append('<div id="second_modal_background"></div>');
  $("body").append('<div id="third_modal_background"></div>');
  $(document).keyup(function(e) {
    if (e.keyCode == 27) {
      do_close_modal(g_modal_dialogs[g_modal_dialogs.length - 1],
                     ['#modal_background',
                      '#second_modal_background',
                      '#third_modal_background'][g_modal_dialogs.length - 1]);
    }
  });
});

// A stack of selectors for the currently-open modal dialog(s), if any.
var g_modal_dialogs = []

function do_show_modal(modal_selector, focus, submit_handler, background_selector, z_index) {
  if (background_selector) {
    // background_selector == false leaves the background in place, if we're
    // going to display another modal immediately after closing this one.
    // Otherwise there's a race between fading in the background for the new
    // modal and fading out the background for the closing modal.
    var modal_background = $(background_selector);
    modal_background.css({'display': 'block',
                          'opacity': 0});
    modal_background.fadeTo(200, 0.5);
  }

  var modal_div = $(modal_selector);
  if (typeof submit_handler != 'undefined' && submit_handler) {
    var form = modal_div.find("form");
    form.off("submit");
    form.on("submit", function(event) {
      event.preventDefault();
      submit_handler(event);
    });
  }

  g_modal_dialogs.push(modal_selector);

  if (modal_div.closest('.modal_frame').length == 0) {
    modal_div.wrap('<div class="modal_frame"></div>');
    modal_div.closest('.modal_frame').css({ 'z-index': z_index });
  }
  modal_div.closest('.modal_frame').removeClass('hidden');

  var modal_width = modal_div.outerWidth();
  modal_div.removeClass("hidden");
  modal_div.css({ 
    'display': 'block',
    'position': 'absolute',
    'opacity': 0,
    'left' : 50 + '%',
    'margin-left': -(modal_width/2) + "px"
  });
  modal_div.fadeTo(200, 1);
  if (focus) {
    setTimeout(function() { $(focus).focus(); }, 210);
  }
}

function do_close_modal(modal_selector, background_selector) {
  g_modal_dialogs.pop();
  if (background_selector) {
    $(background_selector).fadeOut(200);
  }
  $(modal_selector).closest('.modal_frame')
    .addClass('hidden')
    // .css({'display': 'none'})
    ;
}

function show_modal(modal_selector, focus, submit_handler) {
  if (arguments.length == 2 && Object.prototype.toString.call(focus) == "[object Function]") {
    submit_handler = focus;
    focus = null;
  }
  do_show_modal(modal_selector, focus, submit_handler,
                "#modal_background", 11000);
}

function close_modal(modal_selector) {
  do_close_modal(modal_selector, "#modal_background");
}

function close_modal_leave_background(modal_selector) {
  do_close_modal(modal_selector, false);
}

// If we want to morph one dialog into another, at the same level; everything's
// the same except as do_close_modal for fading the background.  Because the
// background's not affected, the level of the modal doesn't matter.
// hide_modal() should be followed immediately with another show_modal call of
// some kind.
function hide_modal(modal_selector) {
  g_modal_dialogs.pop();
  $(modal_selector).closest('.modal_frame').addClass('hidden');
  $(modal_selector).css({'display': 'none'});
}

// For a modal that's supposed to appear in front of another ("ordinary") modal.
function show_secondary_modal(modal_selector, focus, submit_handler) {
  if (arguments.length == 2 && Object.prototype.toString.call(focus) == "[object Function]") {
    submit_handler = focus;
    focus = null;
  }
  do_show_modal(modal_selector, focus, submit_handler,
                "#second_modal_background", 13000);
}

function close_secondary_modal(modal_selector) {
  do_close_modal(modal_selector, "#second_modal_background");
}

function show_tertiary_modal(modal_selector, focus, submit_handler) {
  if (arguments.length == 2 && Object.prototype.toString.call(focus) == "[object Function]") {
    submit_handler = focus;
    focus = null;
  }
  do_show_modal(modal_selector, focus, submit_handler,
                "#third_modal_background", 15000);
}

function close_tertiary_modal(modal_selector) {
  do_close_modal(modal_selector, "#third_modal_background");
}

function push_modal(modal_selector, focus, submit_handler) {
  do_show_modal(modal_selector, focus, submit_handler,
                ["#modal_background",
                 "#second_modal_background",
                 "#third_modal_background"][g_modal_dialogs.length],
                11000 + 2000 * g_modal_dialogs.length);
}

function pop_modal() {
  var modal_selector = g_modal_dialogs[g_modal_dialogs.length - 1];
  do_close_modal(modal_selector,
                 ["", "#modal_background",
                  "#second_modal_background",
                  "#third_modal_background"][g_modal_dialogs.length]);
}
