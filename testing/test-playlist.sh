#! /bin/bash

BASE_URL=$1
set -e -E -o pipefail
source `dirname $0`/common.sh

user_login_coordinator

`dirname $0`/reset-database.sh $BASE_URL

# $1 = car number
# $2 = classname
function make_racer() {
    curl_postj action.php \
              "action=json.racer.import&firstname=First$1&lastname=Last$1&classname=$2&carnumber=$1" \
        | check_jsuccess
}

make_racer 101 Den1
make_racer 102 Den1
make_racer 201 Den2
make_racer 202 Den2
make_racer 203 Den2
make_racer 301 Den3
make_racer 302 Den3

curl_postj action.php "action=json.class.add&name=TwoThree&constituent_2&constituent_3" | check_jsuccess

curl_postj action.php "action=json.racer.bulk&who=all&what=checkin" | check_jsuccess

KIOSK1=FAKE-KIOSK1

curl_getj "action.php?query=json.poll.kiosk&address=$KIOSK1" | expect_one identify.kiosk
curl_postj action.php "action=json.kiosk.assign&address=$KIOSK1&name=Main" | check_jsuccess
curl_postj action.php "action=json.kiosk.assign&address=$KIOSK1&page=kiosks/welcome.kiosk" | check_jsuccess
curl_getj "action.php?query=json.poll.kiosk&address=$KIOSK1" | expect_one welcome.kiosk

# Assuming Den1's first round is roundid = 1, etc.
#
# Assuming sceneid 4 is "Racing"  with now-racing page on Main,
# and sceneid 5 is "Awards", with awards-presentation page on Main

curl_post action.php "action=settings.write&unused-lane-mask=0&n-lanes=2" | check_success
curl_post action.php "action=settings.write&racing_scene=4" | check_success

curl_postj action.php "action=json.schedule.generate&roundid=1" | check_jsuccess

curl_postj action.php "action=json.playlist.add&classid=1&round=1&sceneid_at_finish=5" | check_jsuccess
curl_postj action.php "action=json.playlist.add&classid=2&round=1&n_times_per_lane=1&continue_racing=1" | check_jsuccess
curl_postj action.php "action=json.playlist.add&classid=3&round=1&n_times_per_lane=1&continue_racing=1" | check_jsuccess
curl_postj action.php "action=json.playlist.add&classid=4&round=1&top=3&bucketed=0&n_times_per_lane=2" | check_jsuccess

# Race roundid=1:
curl_postj action.php "action=json.heat.select&roundid=1&now_racing=1" | check_jsuccess
curl_getj "action.php?query=json.poll.kiosk&address=$KIOSK1" | expect_one now-racing.kiosk

curl_post action.php "action=timer-message&message=STARTED" | check_success
curl_post action.php "action=timer-message&message=FINISHED&lane1=1.00&lane2=2.00" | check_success
curl_post action.php "action=timer-message&message=STARTED" | check_success
curl_post action.php "action=timer-message&message=FINISHED&lane1=1.00&lane2=2.00" | check_success
# After the first round, we should have Den2 scheduled and teed up, but not
# racing.  After a brief pause, we should see the scene switched to Awards
echo "Waiting for scene change to take effect..."
sleep 11s
curl_getj "action.php?query=json.poll.kiosk&address=$KIOSK1" | expect_one award-presentations.kiosk

curl_getj "action.php?query=json.poll.coordinator" | \
    jq ".[\"current-heat\"] | .[\"now_racing\"] == false and .roundid == 2" | \
    expect_eq true

curl_postj action.php "action=json.kiosk.assign&address=$KIOSK1&page=kiosks/flag.kiosk" | check_jsuccess

# Race roundid=2:
curl_postj action.php "action=json.heat.select&now_racing=1" | check_jsuccess
curl_getj "action.php?query=json.poll.kiosk&address=$KIOSK1" | expect_one now-racing.kiosk

curl_post action.php "action=timer-message&message=STARTED" | check_success
curl_post action.php "action=timer-message&message=FINISHED&lane1=1.00&lane2=1.20" | check_success
curl_post action.php "action=timer-message&message=STARTED" | check_success
curl_post action.php "action=timer-message&message=FINISHED&lane1=1.00&lane2=1.30" | check_success
curl_post action.php "action=timer-message&message=STARTED" | check_success
curl_post action.php "action=timer-message&message=FINISHED&lane1=1.00&lane2=1.40" | check_success
# No scene change, move right into round 3
curl_getj "action.php?query=json.poll.kiosk&address=$KIOSK1" | expect_one now-racing.kiosk

curl_getj "action.php?query=json.poll.coordinator" | \
    jq ".[\"current-heat\"] | .[\"now_racing\"] == true and .roundid == 3" | \
    expect_eq true

curl_post action.php "action=timer-message&message=STARTED" | check_success
curl_post action.php "action=timer-message&message=FINISHED&lane1=1.00&lane2=1.20" | check_success
curl_post action.php "action=timer-message&message=STARTED" | check_success
curl_post action.php "action=timer-message&message=FINISHED&lane1=1.00&lane2=1.40" | check_success

# No scene change, move right into round 4, which picks a roster
curl_getj "action.php?query=json.poll.kiosk&address=$KIOSK1" | expect_one now-racing.kiosk

curl_getj "action.php?query=json.poll.coordinator" | \
    jq ".[\"current-heat\"] | .[\"now_racing\"] == true and .roundid == 4" | \
    expect_eq true

# First heat: 203 v. 201
curl_getj "action.php?query=json.poll.coordinator" | \
    jq ".racers | all((.lane == 1 and .carnumber == 203) or 
                      (.lane == 2 and .carnumber == 201))" | \
    expect_eq true
curl_post action.php "action=timer-message&message=STARTED" | check_success
curl_post action.php "action=timer-message&message=FINISHED&lane1=1.00&lane2=1.20" | check_success
# Second heat: 302 v. 203
curl_getj "action.php?query=json.poll.coordinator" | \
    jq ".racers | all((.lane == 1 and .carnumber == 302) or 
                      (.lane == 2 and .carnumber == 203))" | \
    expect_eq true
curl_post action.php "action=timer-message&message=STARTED" | check_success
curl_post action.php "action=timer-message&message=FINISHED&lane1=1.00&lane2=1.20" | check_success
# Third heat: 201 v. 302
curl_getj "action.php?query=json.poll.coordinator" | \
    jq ".racers | all((.lane == 1 and .carnumber == 201) or 
                      (.lane == 2 and .carnumber == 302))" | \
    expect_eq true
