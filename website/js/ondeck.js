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


function repopulate_schedule(json) {
  var nlanes = g_nlanes;
  var interleaved = json['current-heat']['use_master_sched'];

  function add_byes(row, nbyes) {
    while (nbyes > 0) {
      $("<td class='bye'/>").text("Bye").appendTo(row);
      --nbyes;
    }
  }

  function car_photo_url(render, elements) {
    return 'photo.php/car/file/' + render + '/' +
      elements['basename'] + '/' + elements['cache-breaker'];
  }

  $("table#schedule tr").remove();

  var th_width_vh = interleaved ? 10 : 4;
  var td_width_vh = (100 - th_width_vh) / (nlanes || 1);

  // $("table#schdule").width() is zero if the table is unpopulated
  // 26 = 2 * 13px padding
  // var photo_width_px = Math.floor($('html')[0].clientWidth  * td_width_vh / 100) - 26;
  // var photo_height_px = 100;
  // var rendername = photo_width_px + 'x' + photo_height_px;
  var rendername = '200x200';  // RENDER_ONDECK

  // roundid and heat of the tr currently under construction
  var roundid = 0;
  var heat = 0;
  var lane = 0;
  var rowno = 0;
  var row_has_photos = false;
  var racerids = Array(nlanes).fill(null);
  var prev_racerids = Array(nlanes).fill(null);
  for (var c = 0; c < json['ondeck']['chart'].length; ++c) {
    var cell = json['ondeck']['chart'][c];
    if (cell['roundid'] != roundid || cell['heat'] != heat) {
      if (row) {
        add_byes(row, nlanes - lane);
        row.toggleClass('populated', row_has_photos);
      }
      var row = $("<tr/>").appendTo("table#schedule");
      ++rowno;
	  //new heat reset lane count to zero to ensure correct byes are populated
	  lane = 0;
      row_has_photos = false;
      prev_racerids = racerids;
      racerids = Array(nlanes).fill(null);
      row.addClass('heat_row')
         .addClass('d' + (rowno % 2))
        .attr('id', 'heat_' + cell['roundid'] + '_' + cell['heat']);
      $("<th/>").text(interleaved
                      ? cell['class'] + ", Heat " + cell['heat']
                      : "Heat " + cell['heat'])
        .css({'width': th_width_vh + 'vh'})
        .appendTo(row);
      roundid = cell['roundid'];
      heat = cell['heat'];
    } 
    add_byes(row, cell['lane'] - 1 - lane);

    var td = $("<td/>").appendTo(row).css({'width': td_width_vh + 'vh'});
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
                .attr('data-img', car_photo_url('WORK', cell['carphoto']))
                .attr('onclick', 'handle_photo_click(this);'));
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


function animate_next_heat() {
  var curheat = $(".curheat");
  var nextheat = $(".nextheat");
  var nextnext = $("tr.nextheat").next();
  var duration = 3000;
  // From the css, the height of populated ondeck_photo divs
  var photo_height_px = 100;
  // Height of div.time
  var time_height_px = 20;

  var over = $("<div/>").appendTo('body');

  // Promises would be cleaner, but won't work on older browsers.
  function phase1() {
    $(".curheat .time")
      .css({'height': '0', 'display': 'block'})
      .animate({'height': '20px'},
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

    if (nextnext.length != 0) {
      var completed_one = false;
      nextnext.find(".ondeck_photo")
        .css({'display': 'block',
              'height': 0})
        .animate({'height': nextnext.hasClass('populated') ? photo_height_px : 0},
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
      json['ondeck']['chart'].length == $("table#schedule td:not(.bye)").length) {
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

  setInterval(function() {
    if (!running) {
      running = true;
      $.ajax('action.php',
             {type: 'GET',
              data: {query: 'poll',
                     values: 'ondeck,current-heat'},
              success: function(json) {
                if (g_resized) {
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
  },
              1000);
});

$(window).on('resize', function() {
  console.log('resized!');
  g_resized = true; });

