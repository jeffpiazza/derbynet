#! /bin/bash

BASE_URL=$1
set -e -E -o pipefail
source `dirname $0`/common.sh

user_login_coordinator

# nlanes=6, to count physical lanes
curl_post action.php "action=settings.write&n-lanes=6" | check_success

### Check in every other racer...
`dirname $0`/test-basic-checkins.sh "$BASE_URL"


# Make six lanes down to 4, 100001 = 33
#  (Also confirm that max-runs-per-car=0, as opposed to unset, works as expected.)
curl_post action.php "action=settings.write&unused-lane-mask=33&max-runs-per-car=0" | check_success
curl_post action.php "action=schedule.generate&roundid=1" | check_success
curl_post action.php "action=heat.select&roundid=1&now_racing=0" | check_success
curl_json "action.php?query=json.poll.coordinator" | \
    jq ".racers | map(select(.lane == 1 or .lane == 6)) | length" | \
    expect_eq 0
staged_heat6 - 101 121 141 111 -
curl_post action.php "action=heat.select&heat=next" | check_success
staged_heat6 - 111 131 101 121 -
curl_post action.php "action=heat.select&heat=next" | check_success
staged_heat6 - 121 141 111 131 -
curl_post action.php "action=heat.select&heat=next" | check_success
staged_heat6 - 131 101 121 141 -
curl_post action.php "action=heat.select&heat=next" | check_success
staged_heat6 - 141 111 131 101 -


# Use every other lane, 101010 = 42
# Writing settings with unraced schedule will fail:
curl_post action.php "action=settings.write&unused-lane-mask=42" | check_failure
# Previously we selected different heats but didn't race any of them, so the
# schedule can just be removed.
curl_post action.php "action=schedule.unschedule&roundid=1" | check_success

curl_post action.php "action=settings.write&unused-lane-mask=42" | check_success
curl_post action.php "action=schedule.generate&roundid=2" | check_success
curl_post action.php "action=heat.select&roundid=2&now_racing=0" | check_success
curl_json "action.php?query=json.poll.coordinator" | \
    jq ".racers | map(select(.lane == 2 or .lane == 4 or .lane == 6)) | length" | \
    expect_eq 0
staged_heat6 207 - 227 - 247 -
curl_post action.php "action=heat.select&heat=next" | check_success
staged_heat6 217 - 237 - 207 -
curl_post action.php "action=heat.select&heat=next" | check_success
staged_heat6 227 - 247 - 217 -
curl_post action.php "action=heat.select&heat=next" | check_success
staged_heat6 237 - 207 - 227 -
curl_post action.php "action=heat.select&heat=next" | check_success
staged_heat6 247 - 217 - 237 -



