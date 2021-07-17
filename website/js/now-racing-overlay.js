var Overlay = {
  overlay_shown: '',

  clear: function() {
    $(".overlay_foreground").fadeTo(200, 0);
    $("#overlay_background").fadeTo(200, 0);
    this.overlay_shown = '';
  },

  show: function(selector) {
    if (this.overlay_shown != selector) {
      var background = $("#overlay_background");
      background.css({'display': 'block',
                      'opacity': 0});
      background.fadeTo(200, 0.5);
      $(".overlay_foreground").css('opacity', 0);

      var overlay = $(selector);
      overlay.css({
        'display': 'block',
        'position': 'fixed',
        'opacity': 0,
        'z-index': 11000,
        'left' : '50%',
        'margin-left': '-256px',
        'top': $(window).height() / 2 + "px",
        'margin-top': '-256px'
      });
      overlay.fadeTo(200, 1);
      this.overlay_shown = selector;
    }
  }
};

