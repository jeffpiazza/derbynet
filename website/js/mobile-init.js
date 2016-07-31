// We're using jQuery Mobile for its nice mobile/tablet-friendly UI
// elements.  By default, jQM also wants to hijack page loading
// functionality, and perform page loads via ajax, a "service" we
// don't really want.  Fortunately, it's possible to turn that stuff
// off.
$(document).bind("mobileinit", function() {
                                   $.extend($.mobile, {
                                         ajaxEnabled: false
                                         });
                                 });
