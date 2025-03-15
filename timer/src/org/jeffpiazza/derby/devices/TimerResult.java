package org.jeffpiazza.derby.devices;

import java.util.ArrayList;
import org.jeffpiazza.derby.Message;

public class TimerResult {
  private int laneMask;
  private ArrayList<Message.LaneResult> results;

  public TimerResult(int laneMask) {
    this.laneMask = laneMask;
    int lane_count = 32 - Integer.numberOfLeadingZeros(laneMask);
    results = new ArrayList<Message.LaneResult>(lane_count);
    for (int lane = 0; lane < lane_count; ++lane) {
      results.add(
          ((laneMask & (1 << lane)) != 0) ? new Message.LaneResult() : null);
    }
  }

  // lane is 1-based
  public void setLane(int lane, String time) {
    setLane(lane, time, 0);
  }

  // lane and place are 1-based; place=0 for no place value
  public void setLane(int lane, String time, int place) {
    if (lane == 0) {
      // NewBold (at least) sends a lane 0 result of 0.000 for DNFs.
      // We assume that means there won't be further valid times that follow.
      // We need to clear one bit in the laneMask, doesn't matter which one, so
      // that the caller will generate a RACE_FINISHED event when all the
      // "results" are in.  This clears the lowest set bit in the mask:
      laneMask &= ~(laneMask & -laneMask);
    } else {
      setLaneZeroBased(lane - 1, time, place);
    }
  }

  // lane is 0-based, but place remains 1-based
  private void setLaneZeroBased(int lane, String time, int place) {
    if (lane >= 0 && lane < results.size()) {
      Message.LaneResult r = results.get(lane);
      if (r != null) {
        r.time = time;
        r.place = place;
      }
      laneMask &= ~(1 << lane);
    }
  }

  public int mask() { return laneMask; }  // For debugging

  public boolean isFilled() {
    return laneMask == 0;
  }

  public Message.LaneResult[] toArray() {
    return results.toArray(new Message.LaneResult[results.size()]);
  }
}
