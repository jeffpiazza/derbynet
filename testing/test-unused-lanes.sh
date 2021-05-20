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
curl_postj action.php "action=json.schedule.generate&roundid=1" | check_jsuccess
curl_postj action.php "action=json.heat.select&roundid=1&now_racing=0" | check_jsuccess
curl_getj "action.php?query=json.poll.coordinator" | \
    jq ".racers | map(select(.lane == 1 or .lane == 6)) | length" | \
    expect_eq 0
staged_heat6 0 101 121 141 111 0
curl_postj action.php "action=json.heat.select&heat=next" | check_jsuccess
staged_heat6 0 111 131 101 121 0
curl_postj action.php "action=json.heat.select&heat=next" | check_jsuccess
staged_heat6 0 121 141 111 131 0
curl_postj action.php "action=json.heat.select&heat=next" | check_jsuccess
staged_heat6 0 131 101 121 141 0
curl_postj action.php "action=json.heat.select&heat=next" | check_jsuccess
staged_heat6 0 141 111 131 101 0


# Use every other lane, 101010 = 42
# Writing settings with unraced schedule will fail:
curl_post action.php "action=settings.write&unused-lane-mask=42" | check_failure
# Previously we selected different heats but didn't race any of them, so the
# schedule can just be removed.
curl_postj action.php "action=json.schedule.unschedule&roundid=1" | check_jsuccess

curl_post action.php "action=settings.write&unused-lane-mask=42" | check_success
curl_postj action.php "action=json.schedule.generate&roundid=2" | check_jsuccess
curl_postj action.php "action=json.heat.select&roundid=2&now_racing=0" | check_jsuccess
curl_getj "action.php?query=json.poll.coordinator" | \
    jq ".racers | map(select(.lane == 2 or .lane == 4 or .lane == 6)) | length" | \
    expect_eq 0
staged_heat6 207 0 227 0 247 0
curl_postj action.php "action=json.heat.select&heat=next" | check_jsuccess
staged_heat6 217 0 237 0 207 0
curl_postj action.php "action=json.heat.select&heat=next" | check_jsuccess
staged_heat6 227 0 247 0 217 0
curl_postj action.php "action=json.heat.select&heat=next" | check_jsuccess
staged_heat6 237 0 207 0 227 0
curl_postj action.php "action=json.heat.select&heat=next" | check_jsuccess
staged_heat6 247 0 217 0 237 0



