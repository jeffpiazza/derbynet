// table#main-table is the main racing table

var FontAdjuster = {
  last_table_bottom: 0,
  last_table_right: 0,
  original_font_size: 0,

  start: function() {
    this.table_resized();
    setInterval(function () { FontAdjuster.adjust_fonts(); }, 75);
  },

  // When the table has new content, try going back to the original font size
  // and then shrink as necessary.
  reset: function() {
    $("#main-table td").css('font-size', this.original_font_size);
  },

  table_resized: function() {
    var cells = $("#main-table td.photo");
    var row_height =
        Math.floor(($(window).height() - cells.position().top) / cells.length) -
        this.parse_px(cells.css('border-bottom-width'));

    // 1.3 here is the line height used throughout the page (from global.css)
    this.original_font_size = row_height / 1.3;
  },

  adjust_fonts: function() {
    this.adjust_table_fonts();
    this.adjust_banner_font();
  },

  adjust_banner_font: function() {
    var title = $(".banner .banner_title");
    var client_rect = title.get(0).getBoundingClientRect();
    if (client_rect.height > 60) {
      title.css('font-size', Math.round(0.90 * this.parse_px(title.css('font-size'))) + 'px');
    }
  },
  
  adjust_table_fonts: function() {
    var client_rect = document.getElementById("main-table").getBoundingClientRect();

    if (client_rect.bottom != this.last_table_bottom) {
      this.last_table_bottom = client_rect.bottom;
      var border_bottom_width = this.parse_px($("#main-table td").css('border-bottom-width'));
      if (client_rect.bottom > $(window).height() + border_bottom_width / 2) {
        $("#main-table td").css('font-size', Math.round(0.90 * this.font_size()) + "px");
      }
    }

    // Under some circumstances, the table displays too wide for a narrow
    // window, even if it fits vertically.
    if (client_rect.right != this.last_table_right) {
      this.last_table_right = client_rect.right;
      var border_right_width = this.parse_px($("#main-table td").css('border-right-width'));
      if (client_rect.right > $(window).width() + border_right_width / 2) {
        $("#main-table td").css('font-size', Math.round(0.90 * this.font_size()) + "px");
      }
    }
  },

  font_size: function() {
    return this.parse_px($("#main-table td").css('font-size'));
  },

  parse_px: function(css_value) {
    // css_value is expected to end in "px", so we strip that off.
    return parseInt(css_value.substring(0, css_value.length - 2), 10);
  }
};

$(function() { FontAdjuster.start(); });
