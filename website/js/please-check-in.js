$(function() {
  var scrollables = [];
  $(".kiosk_wanted ul").each(function(index, element) {
    if (element.getBoundingClientRect().bottom > $(window).height()) {
      scrollables.push($(element));
    }
  });
  if (scrollables.length > 0) {
    setInterval(function() {
      $.each(scrollables, function(index, ul) {
        var child = ul.children().first();
        var height = child.height();
        child.animate({height: '0px'}, 400,
                      function() { child.height(height).detach().appendTo(ul); });
      });
    }, 1500);
  }
});
