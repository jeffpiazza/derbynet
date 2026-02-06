// This poller overrides the regular now-racing poller to request phonetic names

var g_row_height;
$(function () {
  Poller.build_request = function (roundid, heat) {
    // TODO It shouldn't be necessary to send row-height to the server;
    // instead just construct a racer photo URL from the returned racerid.
    g_row_height = 0;
    var photo_cells = $("td.photo");
    var border = parseInt(photo_cells.css("border-bottom-width"));

    if (photo_cells.length > 0) {
      // Position of the first td.photo may get adjusted
      g_row_height =
        Math.floor(
          ($(window).height() - photo_cells.position().top) / photo_cells.length
        ) - border;
    }

    return {
      query: "poll",
      values:
        "current-heat,heat-results,precision,racers-phonetic," +
        "timer-trouble,current-reschedule",
      roundid: roundid,
      heat: heat,
      "row-height": g_row_height,
    };
  };
});
