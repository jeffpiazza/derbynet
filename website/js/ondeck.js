// These globals need to be provided from the context in which this file is loaded:
//
// var g_nlanes
// var g_show_car_photos
// var g_set_nextheat


function handle_row_click(tr) {
  $(tr).toggleClass('exposed');
}

function handle_photo_click(img) {
  $("#photo_view_img").attr('src', $(img).attr('data-img'));
  show_modal("#photo_view_modal", function() {});
}

// These globals are computed by repopulate_schedule when the cell sizes are
// calculated.
var g_img_max_width_px;
var g_img_max_height_px;

function repopulate_schedule(json) {
  var nlanes = g_nlanes;
  var interleaved = json['current-heat']['use_master_sched'];

  var th0_width_vw, th_width_vw;
  if (interleaved) {
    th0_width_vw = 5;
    th_width_vw = 10;
  } else {
    th0_width_vw = 0;
    th_width_vw = 4;
  }
  var td_width_vw = (100 - (th0_width_vw + th_width_vw) - 5 /* border */) / (nlanes || 1);
  // CSS specifies td padding 0.5vw, so max width for each car photo is td_width_vw - 1 (vw).
  g_img_max_width_px = window.innerWidth * ((td_width_vw - 1) / 100);
  // Don't let any super-tall, thin image screw things up completely; limit to
  // 25% of viewport height.
  g_img_max_height_px = window.innerHeight * 25 / 100;

  var rounds_map = {};
  for (var i = 0; i < json['rounds'].length; ++i) {
    rounds_map[json['rounds'][i]['roundid']] = json['rounds'][i];
  }

  function add_byes(row, nbyes) {
    while (nbyes > 0) {
      $("<td class='bye'/>").text("Bye").appendTo(row).css({'width': td_width_vw + 'vw'});
      --nbyes;
    }
  }

  function car_photo_url(render, elements) {
    return 'photo.php/car/file/' + render + '/' +
      elements['basename'] + '/' + elements['cache-breaker'];
  }

  $("table#schedule tr").remove();

  {
    // The first row sets the column widths, at least on some browsers, so we
    // need an invisible sizing row before the divider
    var sizer = $("<tr style='height: 0; border: none;'/>").appendTo('table#schedule');
    if (interleaved) {
      sizer.append($('<th/>').css({'width': th0_width_vw + 'vw', 'height': 0}));
    }
    sizer.append($('<th/>').css({'width': th_width_vw + 'vw', 'height': 0}));
    for (var lane = 0; lane < nlanes; ++lane) {
      sizer.append($('<td/>').css({'width': td_width_vw + 'vw',
                                   'height': 0,
                                   'padding': 0,
                                   'border': 'none'}));
    }
  }

  var rendername = '400x400';  // RENDER_ONDECK

  // roundid and heat of the tr currently under construction
  var roundid = 0;
  var heat = 0;
  var lane = 0;
  var row;
  var rowno = 0;
  var row_has_photos = false;
  var racerids = Array(nlanes).fill(null);
  var prev_racerids = Array(nlanes).fill(null);
  for (var c = 0; c < json['ondeck']['chart'].length; ++c) {
    var cell = json['ondeck']['chart'][c];
    if (cell['roundid'] != roundid || cell['heat'] != heat) {
      if (cell['roundid'] != roundid && !interleaved) {
        $("<tr/>").appendTo('table#schedule')
          .append($("<th class='divider'/>")
                  .attr('colspan', nlanes + 1)
                  .text(rounds_map[cell['roundid']]['name']));
      }
      if (row) {
        add_byes(row, nlanes - lane);
        row.toggleClass('populated', row_has_photos);
      }
      row = $("<tr/>").appendTo("table#schedule");
      ++rowno;
	  //new heat reset lane count to zero to ensure correct byes are populated
	  lane = 0;
      row_has_photos = false;
      prev_racerids = racerids;
      racerids = Array(nlanes).fill(null);
      row.addClass('heat_row')
         .addClass('d' + (rowno % 2))
        .attr('id', 'heat_' + cell['roundid'] + '_' + cell['heat']);
      var row_label = "Heat " + cell['heat'];
      if (interleaved) {
        row_label = rounds_map[cell['roundid']]['name'] + ', ' + row_label;
        $("<th class='masterheat'/>").text(cell['masterheat'])
          .css({'width': th0_width_vw + 'vw'})
          .appendTo(row);
      }
      $("<th/>").text(row_label)
        .css({'width': th_width_vw + 'vw'})
        .appendTo(row);
      roundid = cell['roundid'];
      heat = cell['heat'];
    } 
    add_byes(row, cell['lane'] - 1 - lane);

    var td = $("<td class='chart'/>").appendTo(row).css({'width': td_width_vw + 'vw'});
    td.addClass('lane_' + cell['lane'])
      .addClass('resultid_' + cell['resultid']);
    if (prev_racerids.indexOf(cell['racerid']) >= 0) {
      td.addClass('in_prev');
    }
    racerids[cell['lane']] = cell['racerid'];
    td.append($('<div/>').addClass('car').text(cell['carnumber']))
      .append($('<div/>').addClass('racer').text(cell['name']))
      .append($('<div/>').addClass('time').text(cell['result'].substring(1))
              .css('display', cell['result'] ? 'block' : 'none'));

    var photo_div = $("<div/>").addClass('ondeck_photo').appendTo(td);
    if (g_show_car_photos && cell['carphoto']) {
      photo_div
        .append($("<img/>")
                .attr('src', car_photo_url(rendername, cell['carphoto']))
                .attr('data-img', car_photo_url('work', cell['carphoto']))
                .on('click', function() { handle_photo_click(this); })
                .on('load', function() {
                  var row_photo_divs = $(this).closest('tr').find('div.ondeck_photo');
                  // On load, adjust all the div.ondeck_photo in the row to be
                  // the same height.  These are only intended for visible
                  // images; animate_next_heat will cancel these handlers to
                  // avoid the risk they'll screw up the animation (even though
                  // images loaded from cache typically don't fire the load
                  // handler).
                  var max_height = 0;
                  row_photo_divs.each(function() {
                    max_height = Math.max(max_height, $(this).height());
                  });
                  row_photo_divs.css({height: max_height});
                })
                .css({'max-width': g_img_max_width_px + 'px',
                      'max-height' : g_img_max_height_px + 'px'}));
      row_has_photos = true;
    }
    lane = cell['lane'];
  }

  if (row) {
    add_byes(row, nlanes - lane);
    row.toggleClass('populated', row_has_photos);
  }

  $("table#schedule tr#heat_" + json['current-heat']['roundid'] +
    '_' + json['current-heat']['heat']).addClass('curheat');

  if (json['ondeck']['next'] && g_set_nextheat) {
    $("table#schedule tr#heat_" + json['ondeck']['next']['roundid'] +
      '_' + json['ondeck']['next']['heat']).addClass('nextheat');
  }
}

function preload_images_for_tallest_aspect(imgs_jquery, ondone) {
  var max_aspect = 0;
  var n_waiting = 0;

  imgs_jquery.each(function(i, elt) {
    var image = new Image();
    image.onload = function() {
      --n_waiting;
      if (image.width != 0) {
        max_aspect = Math.max(max_aspect, image.height / image.width);
      }
      image.remove();
      if (n_waiting <= 0) {
        ondone(max_aspect);
      }
    };
    image.src = $(elt).attr('src');
  });
}

function animate_next_heat() {
  var curheat = $(".curheat");
  var nextheat = $(".nextheat");
  var nextnext = $("tr.nextheat").next();
  var duration = 3000;  // ms.

  // Height of a div.time when it's shown.  css says font-size 32px and
  // line-height 1.3 gives 41.6, which the browser rounds down to 41px.  We
  // animate div.time height from 0 to this value when exposing the time.
  var time_height_px = 41;

  // Cancel img load handlers added by repopulate_schedule that might compete
  // with the animation for showing the nextnext row.
  nextnext.find('img').off('load');

  var nextnext_tallest_aspect = 0;
  preload_images_for_tallest_aspect(nextnext.find('img'),
                                    function(aspect) { nextnext_tallest_aspect = aspect; });
  
  // Animated text overlay
  var over = $("<div/>").appendTo('body');

  // Promises would be cleaner, but won't work on older browsers, and it's
  // important to be able to run the on-deck page on old tablets or what have
  // you.
  function phase1() {
    // For each cell in the the just-completed heat, shrink the car photo to
    // zero height, open room for the recorded heat time, and shrink the borders
    // to 1px.  While that's happening, slide a "Just Finished" message across
    // the row, adjusting the overlay font size as the row is changing.
    //
    // Also adjust the nextheat row's border, in anticipation of it becoming the
    // current heat.
    $(".curheat .time")
      .css({'height': '0', 'display': 'block'})
      .animate({'height': time_height_px + 'px'},
               {'duration': duration});
    $(".curheat .ondeck_photo")
      .animate({'height': '0'},
               {'duration': duration,
                'complete': function() {
                  $(".curheat .ondeck_photo").css('display', 'none');
                  $(".curheat").removeClass('curheat');
                }});
    curheat.animate({'border-width': 1},
                    {'duration': duration});
    nextheat.css({'border': '0px black solid'})
      .animate({'border-width': '10px'},
               {'duration': duration});

    var photo_height_px = 0;
    curheat.find('img').each(function() {
      photo_height_px = Math.max(photo_height_px, $(this).height());
    });
    var curheat_height_after = curheat.outerHeight() + time_height_px -
        (curheat.hasClass('populated') ? photo_height_px : 0);
    over.text('Just Finished')
        .css({position: 'absolute',
              'background': 'transparent',
              'font-size': (curheat.outerHeight() * 0.75) + 'px',
              'text-align': 'center',
              'width': curheat.outerWidth(),
              'height': curheat.outerHeight(),
              'text-align': 'left',
              'left': curheat.outerWidth(),
              'top': curheat.offset().top,
             })
        .animate({'height': curheat_height_after,
                  'font-size': curheat_height_after * 0.75,
                  'left': -500,
                 },
                 {'duration': duration,
                  'complete': function() { phase2(); }
                 });
  }

  function phase2() {
    // Switch nextheat to current heat, and fly a "Now Staging" message over the
    // row.  Meanwhile, open the populated ondeck_photo divs to their final height
    curheat.removeClass('curheat');
    if (nextheat.length == 0) {
      nextheat = curheat.next();
    }
    // If .nextheat wasn't assigned, then there was no animation adding its border
    nextheat.removeClass('nextheat').addClass('curheat');

    // Place the overlay text over the nextheat row.
    // nextheat won't change height (it's already opened)
    over.text('Now Staging')
      .css(nextheat.offset())
      .css({'font-size': (nextheat.outerHeight() * 0.75) + 'px',
            'width': nextheat.outerWidth(),
            'height': nextheat.outerHeight(),
            'left': nextheat.outerWidth()
           })
      .animate({'left': -500},
               {'duration': duration,
                'complete': function() { over.remove(); },
               });

    if (nextnext.length != 0 && nextnext.hasClass('populated')) {
      var div_height = Math.min(nextnext_tallest_aspect * g_img_max_width_px, g_img_max_height_px);
      var completed_one = false;
      nextnext.find("div.ondeck_photo")
        .css({'display': 'block',
              'height': 0})
        .animate({'height': div_height},
                 {'duration': duration,
                  'complete': function() {
                    if (completed_one) {
                      return;
                    }
                    completed_one = true;
                    phase3();
                  }
                 });
    } else {
      phase3();
    }
  }

  function phase3() {
    nextnext.addClass('nextheat');
    scroll_to_current_heat();
  }

  phase1();
}


function update_schedule(json, current_heat, next_heat) {
  if (current_heat &&
      json['current-heat']['use_master_sched'] == current_heat['use_master_sched'] &&
      json['current-heat']['roundid'] == current_heat['roundid'] &&
      json['current-heat']['heat'] == current_heat['heat'] &&
      json['ondeck']['chart'].length == $("table#schedule td.chart").length) {
    // No change, do nothing
  } else if (json['current-heat']['roundid'] == next_heat['roundid'] &&
             json['current-heat']['heat'] == next_heat['heat']) {
    // Populate the results for the just-completed heat.
    var cur_roundid = current_heat['roundid'];
    var cur_heat = current_heat['heat'];
    var chart = json['ondeck']['chart'];
    for (var res = 0; res < chart.length; ++res) {
      if (chart[res]['roundid'] == cur_roundid &&
          chart[res]['heat'] == cur_heat) {
        $(".curheat .resultid_" + chart[res]['resultid'] + " .time").text(chart[res]['result'].substring(1));
      }
    }
    animate_next_heat();
  } else {
    repopulate_schedule(json);
    scroll_to_current_heat();
  }
}

function scroll_to_current_heat() {
  var curheat = $(".curheat");
  var nextheat = $(".nextheat");
  if (nextheat.length == 0) {
    nextheat = curheat;
  }
  if (curheat.size() != 0) {
    $("html, body").animate(
      {'scrollTop':  window.scrollY
       + (curheat[0].getBoundingClientRect().top
          + nextheat[0].getBoundingClientRect().bottom) / 2
       - $(window).height()/2 },
      {'duration': 1000});
  }
}

var g_resized = true;

$(function() {
  // Whenever an interval comes around and we're still processing the last one,
  // just skip a cycle.
  var running = false;
  var current_heat;
  var next_heat;
  var interval;

  function poll_for_chart() {
    if (!running) {
      running = true;
      $.ajax('action.php',
             {type: 'GET',
              data: {query: 'poll',
                     values: 'ondeck,rounds,current-heat,current-reschedule'},
              success: function(json) {
                if (g_resized) {
                  if (json["cease"]) {
                    clearInterval(interval);
                    window.location.href = '../index.php';
                    return;
                  }
                  g_resized = false;
                  repopulate_schedule(json);
                  current_heat = json['current-heat'];
                  next_heat = json['ondeck']['next'];
                  scroll_to_current_heat();
                } else {
                  update_schedule(json, current_heat, next_heat);
                }
                current_heat = json['current-heat'];
                next_heat = json['ondeck']['next'];
                running = false;
              }
             });
    }
  }
  interval = setInterval(poll_for_chart, 1000);
  poll_for_chart();
});

$(window).on('resize', function() {
  console.log('resized!');
  g_resized = true; });

