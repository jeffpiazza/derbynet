// We don't know the actual rendered size of the main photo until after it's
// loaded, so that's when we can calculate how much margin to apply to the main
// photo so it's vertically centered.  (If the image is taller than its
// container, margin can be zero and the image will be scaled down to fit.)
function mainphoto_onload(img) {
  var height = img.height;
  if (img.width > $("#photo-background").width()) {
    // img.height and width give the "true" image size, but the photo will
    // actually render at a scaled size
    height = height * $("#photo-background").width() / img.width;
  }
  if (height < $("#photo-background").height()) {
    var top_margin = ($("#photo-background").height() - height) / 2;
    $(img).css('margin-top', top_margin);
  }
  $(img).css({'max-height': $("#photo-background").height(),
             'max-width': $("#photo-background").width()});
}

(function() {
  var current_racer_id = 0;
  // This cache breaker value is for the title slide image.  We want that image
  // to be cached for the duration of the slideshow.  Reloading the slideshow
  // will pick up any changes to the title slide image.
  var cachebreaker = Date.now();
  var kiosk_parameters = {};
  try {
    KioskPoller.param_callback = function(parameters) {
      kiosk_parameters = parameters;
    };
  } catch (e) {
    // If not in a kiosk, then KioskPoller will be undefined.
  }

  function refresh_page(racer) {
    // There's always a div.next, which is hidden; we populate it with images
    // for the next racer.  When the next racer becomes current, the old
    // div.current gets removed, div.next becomes div.current, and we create a
    // new div.next.
    $("#photo-background div.current").remove();
    var current = $("#photo-background div.next");
    current.removeClass("next").addClass("current");
    var next = $("<div class='next'></div>").appendTo("#photo-background");
    if (racer) {
      if (current.find("img.mainphoto").length == 0) {
        current.append('<img class="mainphoto" onload="mainphoto_onload(this)" src="' +
                       racer['main-photo'] + '"/>');
      }
      current.find("img.mainphoto").after('<p class="subtitle">' + 
                     '<span class="carno">' + racer.carnumber + '</span>: ' +
                     racer.name +
                     (racer.carname ?
                      '<br/><i>' + racer.carname + '</i>' : '') +
                     '</p>');
      if (racer.hasOwnProperty('inset-photo') &&
          current.find("img.inset_photo").length == 0) {
        current.append('<img class="inset_photo" src="' +
                       racer['inset-photo'] + '"/>');
      }

      // Preload the next image for better display
      if (racer.hasOwnProperty('next-photo')) {
        next.append('<img class="mainphoto" onload="mainphoto_onload(this)" src="' +
                   racer['next-photo'] + '"/>');
      }
      if (racer.hasOwnProperty('next-inset')) {
        next.append('<img class="inset_photo" src="' +
                   racer['next-inset'] + '"/>');
      }
    } else {
      // We assume there's no img.mainphoto under current, because there
      // shouldn't have been any 'next' racer last time.
      // Also assumes g_title_slide defined in main page.
      current.append('<img class="mainphoto" onload="mainphoto_onload(this)"'
                     + ' src="image.php/slideshow_title.png"/>');
      if (kiosk_parameters.title) {
        $('<p class="maintitle"></p>').text(kiosk_parameters.title).appendTo(current);
      }
    }
  }

  function photo_poll() {
    var classids = kiosk_parameters.classids;
    $.ajax('action.php',
           {type: 'GET',
            data: {query: 'photo.next',
                   racerid: current_racer_id,
                   classids: classids && classids.length > 0 ? classids.join(',') : ''},
            success: function(data) {
              if (data.hasOwnProperty('racer')) {
                current_racer_id = data.racer.racerid;
                refresh_page(data.racer);
              } else {
                current_racer_id = 0;
                refresh_page(null);
              }
            }
           }
          );
  }

  $(document).ready(function() {
    $("#photo-background").height($("#photo-background").height() - $("#photo-background").position().top);
    photo_poll();
    setInterval(photo_poll, 10000);
  });
}());
