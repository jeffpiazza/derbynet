#! /bin/bash

BASE_URL=$1
set -e -E -o pipefail
source `dirname $0`/common.sh

user_login_coordinator

`dirname $0`/reset-database.sh $BASE_URL

curl_post action.php "action=settings.write&unused-lane-mask=0&n-lanes=2" | check_success

# $1 = car number
# $2 = classname
function make_racer() {
    curl_post action.php \
              "action=racer.import&firstname=First$1&lastname=Last$1&classname=$2&carnumber=$1" \
        | check_success
}

make_racer 101 Den1
make_racer 102 Den1
make_racer 201 Den2
make_racer 202 Den2
make_racer 301 Den3
make_racer 302 Den3

curl_post action.php "action=racer.bulk&who=all&what=checkin" | check_success

KIOSK1=FAKE-KIOSK1

curl_get "action.php?query=poll.kiosk&address=$KIOSK1" | expect_one kiosks/identify.kiosk
curl_post action.php "action=kiosk.assign&address=$KIOSK1&name=Main" | check_success
curl_post action.php "action=kiosk.assign&address=$KIOSK1&page=kiosks/welcome.kiosk" | check_success
curl_get "action.php?query=poll.kiosk&address=$KIOSK1" | expect_one kiosks/welcome.kiosk

# Assuming Den1's first round is roundid = 1, etc.
#
# Assuming sceneid 4 is "Awards", with awards-presentation kiosk on Main

curl_post action.php "action=schedule.generate&roundid=1" | check_success

curl_post action.php "action=playlist.new&roundid=1&sceneid_at_finish=4" | check_success
curl_post action.php "action=playlist.new&roundid=2&n_times_per_lane=1&continue_racing=1" | check_success
curl_post action.php "action=playlist.new&roundid=3&n_times_per_lane=1" | check_success

# Race roundid=1:
curl_post action.php "action=heat.select&roundid=1&now_racing=1" | check_success

curl_post action.php "action=timer-message&message=STARTED" | check_success
curl_post action.php "action=timer-message&message=FINISHED&lane1=1.00&lane2=2.00" | check_success
curl_post action.php "action=timer-message&message=STARTED" | check_success
curl_post action.php "action=timer-message&message=FINISHED&lane1=1.00&lane2=2.00" | check_success
# After the first round, we should have Den2 scheduled and teed up, but not
# racing, and scene switched to Awards

curl_get "action.php?query=poll.kiosk&address=$KIOSK1" | expect_one kiosks/award-presentations.kiosk
curl_get "action.php?query=poll.coordinator" | grep current-heat | expect_one roundid=.2.
curl_get "action.php?query=poll.coordinator" | grep current-heat | expect_one now-racing=.0.

curl_post action.php "action=kiosk.assign&address=$KIOSK1&page=kiosks/now-racing.kiosk" | check_success

# Race roundid=2:
curl_post action.php "action=heat.select&now_racing=1" | check_success
curl_post action.php "action=timer-message&message=STARTED" | check_success
curl_post action.php "action=timer-message&message=FINISHED&lane1=1.00&lane2=2.00" | check_success
curl_post action.php "action=timer-message&message=STARTED" | check_success
curl_post action.php "action=timer-message&message=FINISHED&lane1=1.00&lane2=2.00" | check_success
# No scene change, move right into round 3
curl_get "action.php?query=poll.kiosk&address=$KIOSK1" | expect_one "kiosks/now-racing.kiosk"
curl_get "action.php?query=poll.coordinator" | grep current-heat | expect_one roundid=.3.
curl_get "action.php?query=poll.coordinator" | grep current-heat | expect_one now-racing=.1.
