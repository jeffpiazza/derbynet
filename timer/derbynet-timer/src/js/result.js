'use strict';

class HeatResult {
  lanemask;
  lane_results;  // {time:, place:}

  constructor(lanemask) {
    this.lanemask = lanemask;
    var count = 32 - this.countLeadingZeros(lanemask);
    this.lane_results = [];
    for (var lane = 0; lane < count; ++lane) {
      if ((lanemask & (1 << lane)) != 0) {
        this.lane_results.push({time: 0, place: 0});
      } else {
        this.lane_results.push(null);
      }
    }
  }

  // Lane is 1-based
  setLane(lane, time, place = 0) {
    if (lane == 0) {
      // NewBold (at least) sends a lane 0 result of 0.000 for DNFs.
      // We assume that means there won't be further valid times that follow.
      // We need to clear one bit in the laneMask, doesn't matter which one, so
      // that the caller will generate a RACE_FINISHED event when all the
      // "results" are in.  This clears the lowest set bit in the mask:
      this.lanemask &= ~(this.lanemask & -this.lanemask);
    } else {
      return this.setLane_0based(lane - 1, time, place = 0);
    }
  }

  setLane_0based(lane0, time, place = 0) {
    if (lane0 >= 0 && lane0 < this.lane_results.length) {
      this.lane_results[lane0] = {time: time, place: place};
      this.lanemask &= ~(1 << lane0);
      return true;
    }
    return false;
  }

  isFilled() {
    return this.lanemask == 0;
  }

  toString() {
    var v = [];
    for (var i = 0; i < this.lane_results.length; ++i) {
      v.push(this.lane_results[i]?.time);
    }
    return v.join();
  }

  countLeadingZeros(x) {
    let y;
    let n = 32;
    y = x >> 16;
    if (y != 0) { n = n - 16; x = y; }
    y = x >> 8;
    if (y != 0) { n = n - 8; x = y; }
    y = x >> 4;
    if (y != 0) { n = n - 4; x = y; }
    y = x >> 2;
    if (y != 0) { n = n - 2; x = y; }
    y = x >> 1;
    if (y != 0) { return n - 2; }
    return n - x;
  }
}
