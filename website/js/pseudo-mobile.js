// jquery-mobile provides some awesome-looking UI elements, but they come with
// some invasive javascript that wants to take over the whole page.  Here are
// some "pseudo-mobile" UI elements with much more modest ambitions.
//
// Assumes we're using jquery.mobile's CSS.

function pseudo_mobile_radio(radio) {
  radio
    .wrap("<div class='ui-radio'/>")
    .before(function() {
      var label = $("label[for='" + $(this).attr('id') + "']");
      label.addClass('ui-btn ui-btn-a ui-corner-all ui-btn-icon-left ui-btn-inherit');
      if ($(this).is(':checked')) {
        label.addClass('ui-radio-on');
      } else {
        label.addClass('ui-radio-off');
      }
      return label;
    })
    .on('change', function() {
      $("input:radio[name='" + $(this).attr('name') + "']").each(function() {
        $("label[for='" + $(this).attr('id') + "']").removeClass('ui-radio-on').addClass('ui-radio-off');
      });
      $("label[for='" + $(this).attr('id') + "']")
        .addClass('ui-radio-on').removeClass('ui-radio-off');
    });

  $("div[data-role='controlgroup']")
    .addClass('ui-controlgroup ui-controlgroup-vertical ui-corner-all')
    .wrapInner("<div class='ui-controlgroup-controls'/>");
  // ui-first-child and ui-last-child in the absence of a controlgroup?
  $("div[data-role='controlgroup']").find("label").first().addClass('ui-first-child');
  $("div[data-role='controlgroup']").find("label").last().addClass('ui-last-child');
}

function pseudo_mobile_select(select) {
  select.wrap("<div class='ui-select'>" +
              "<div class='ui-btn ui-btn-a ui-icon-carat-d ui-btn-icon-right ui-corner-all ui-shadow'" +
              " id='sortorder-button'>" +
              "</div>" +
              "</div>")
    .before(function() {
      return $("<span></span>").text($(this).find("option:selected").text());
    })
    .on("change", function() {
      $(this).prev("span").text($(this).find("option:selected").text());
    });
}

$(function() {
  pseudo_mobile_select($("select"));
  pseudo_mobile_radio($("input:radio"));
});
