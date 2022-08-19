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
              "action=racer.import&firstname=First$1&lastname=Last$1&partition=$2&carnumber=$1" \
        | check_jsuccess
}

make_racer 101 Den1
make_racer 102 Den1
make_racer 201 Den2
make_racer 202 Den2
make_racer 203 Den2
make_racer 301 Den3
make_racer 302 Den3

curl_postj action.php "action=class.add&name=TwoThree&constituent_2&constituent_3" | check_jsuccess

curl_postj action.php "action=racer.bulk&who=all&what=checkin" | check_jsuccess

KIOSK1=FAKE-KIOSK1

curl_getj "action.php?query=poll.kiosk&address=$KIOSK1" | expect_one identify.kiosk
curl_postj action.php "action=kiosk.assign&address=$KIOSK1&name=Main" | check_jsuccess
curl_postj action.php "action=kiosk.assign&address=$KIOSK1&page=kiosks/welcome.kiosk" | check_jsuccess
curl_getj "action.php?query=poll.kiosk&address=$KIOSK1" | expect_one welcome.kiosk

# Assuming Den1's first round is roundid = 1, etc.
#
# Assuming sceneid 4 is "Racing"  with now-racing page on Main,
# and sceneid 5 is "Awards", with awards-presentation page on Main

curl_postj action.php "action=settings.write&unused-lane-mask=0&n-lanes=2" | check_jsuccess
curl_postj action.php "action=settings.write&racing_scene=4" | check_jsuccess

curl_postj action.php "action=schedule.generate&roundid=1" | check_jsuccess

curl_postj action.php "action=playlist.add&classid=1&round=1&sceneid_at_finish=5" | check_jsuccess
curl_postj action.php "action=playlist.add&classid=2&round=1&n_times_per_lane=1&continue_racing=1" | check_jsuccess
curl_postj action.php "action=playlist.add&classid=3&round=1&n_times_per_lane=1&continue_racing=1" | check_jsuccess
curl_postj action.php "action=playlist.add&classid=4&round=1&top=3&bucketed=0&n_times_per_lane=2" | check_jsuccess

# Race roundid=1:
curl_postj action.php "action=heat.select&roundid=1&now_racing=1" | check_jsuccess
curl_getj "action.php?query=poll.kiosk&address=$KIOSK1" | expect_one now-racing.kiosk

run_heat 1 1   101:1.00 102:2.00 - -
run_heat 1 2   102:1.00 101:2.00 - -    x

# After the first round, we should have Den2 scheduled and teed up, but not
# racing.  After a brief pause, we should see the scene switched to Awards
echo "Waiting for scene change to take effect..."
sleep 11
curl_getj "action.php?query=poll.kiosk&address=$KIOSK1" | expect_one award-presentations.kiosk

curl_getj "action.php?query=poll.coordinator" | \
    jq -e ".[\"current-heat\"] | .[\"now_racing\"] == false and .roundid == 2" >/dev/null || \
    test_fails

curl_postj action.php "action=kiosk.assign&address=$KIOSK1&page=kiosks/flag.kiosk" | check_jsuccess

# Race roundid=2:
curl_postj action.php "action=heat.select&now_racing=1" | check_jsuccess
curl_getj "action.php?query=poll.kiosk&address=$KIOSK1" | expect_one now-racing.kiosk

run_heat 2 1   201:1.00 203:1.20 - -
run_heat 2 2   202:1.00 201:1.30 - -
run_heat 2 3   203:1.00 202:1.40 - - x

# No scene change, move right into round 3
curl_getj "action.php?query=poll.kiosk&address=$KIOSK1" | expect_one now-racing.kiosk

curl_getj "action.php?query=poll.coordinator" | \
    jq -e ".[\"current-heat\"] | .[\"now_racing\"] == true and .roundid == 3" >/dev/null || \
    test_fails

run_heat 3 1   301:1.00 302:1.20 - -
run_heat 3 2   302:1.00 301:1.40 - -  x

# No scene change, move right into round 4, which picks a roster
curl_getj "action.php?query=poll.kiosk&address=$KIOSK1" | expect_one now-racing.kiosk

curl_getj "action.php?query=poll.coordinator" | \
    jq -e ".[\"current-heat\"] | .[\"now_racing\"] == true and .roundid == 4" >/dev/null || \
    test_fails

run_heat 4 1   203:1.00 201:1.20 - -
run_heat 4 2   302:1.00 203:1.20 - -
run_heat 4 3   201:1.00 302:1.20 - -  x

# Remove results for one heat from round 2, and one heat from round 4.
# After re-running the round 2 heat, check that advances directly to round 4.
curl_postj action.php "action=result.delete&roundid=2&heat=2" | check_jsuccess
curl_postj action.php "action=result.delete&roundid=4&heat=3" | check_jsuccess
curl_postj action.php "action=heat.select&roundid=2&heat=2&now_racing=1" | check_jsuccess

run_heat 2 2  202:1.01 201:1.30 - -

curl_getj "action.php?query=poll.coordinator" | \
    jq -e ".[\"current-heat\"] | .[\"now_racing\"] == true and .roundid == 4 and .heat == 3" >/dev/null || \
    test_fails
