#! /bin/bash

BASE_URL=$1
set -e -E -o pipefail
source `dirname $0`/common.sh

user_login_coordinator

curl_post action.php "action=settings.write&use-master-sched-checkbox=1&use-master-sched=1" | check_success
curl_post action.php "action=settings.write&n-lanes=4" | check_success

# Two racers from Bears & Freres (but 4 heats, with byes)
curl_post action.php "action=racer.pass&racer=3&value=1" | check_success
curl_post action.php "action=racer.pass&racer=8&value=1" | check_success

# All the Lions and Tigers (17)
curl_post action.php "action=racer.pass&racer=1&value=1" | check_success
curl_post action.php "action=racer.pass&racer=6&value=1" | check_success
curl_post action.php "action=racer.pass&racer=11&value=1" | check_success
curl_post action.php "action=racer.pass&racer=16&value=1" | check_success
curl_post action.php "action=racer.pass&racer=21&value=1" | check_success
curl_post action.php "action=racer.pass&racer=26&value=1" | check_success
curl_post action.php "action=racer.pass&racer=31&value=1" | check_success
curl_post action.php "action=racer.pass&racer=36&value=1" | check_success
curl_post action.php "action=racer.pass&racer=41&value=1" | check_success
curl_post action.php "action=racer.pass&racer=46&value=1" | check_success
curl_post action.php "action=racer.pass&racer=51&value=1" | check_success
curl_post action.php "action=racer.pass&racer=56&value=1" | check_success
curl_post action.php "action=racer.pass&racer=61&value=1" | check_success
curl_post action.php "action=racer.pass&racer=71&value=1" | check_success
curl_post action.php "action=racer.pass&racer=66&value=1" | check_success
curl_post action.php "action=racer.pass&racer=76&value=1" | check_success
curl_post action.php "action=racer.pass&racer=81&value=1" | check_success

# White's Wolves (13)
curl_post action.php "action=racer.pass&racer=2&value=1" | check_success
curl_post action.php "action=racer.pass&racer=7&value=1" | check_success
curl_post action.php "action=racer.pass&racer=12&value=1" | check_success
curl_post action.php "action=racer.pass&racer=22&value=1" | check_success
curl_post action.php "action=racer.pass&racer=17&value=1" | check_success
curl_post action.php "action=racer.pass&racer=27&value=1" | check_success
curl_post action.php "action=racer.pass&racer=32&value=1" | check_success
curl_post action.php "action=racer.pass&racer=37&value=1" | check_success
curl_post action.php "action=racer.pass&racer=42&value=1" | check_success
curl_post action.php "action=racer.pass&racer=52&value=1" | check_success
curl_post action.php "action=racer.pass&racer=62&value=1" | check_success
curl_post action.php "action=racer.pass&racer=72&value=1" | check_success
curl_post action.php "action=racer.pass&racer=82&value=1" | check_success

# Three racers from Webelos (4 heats, with byes)
curl_post action.php "action=racer.pass&racer=39&value=1" | check_success
curl_post action.php "action=racer.pass&racer=44&value=1" | check_success
curl_post action.php "action=racer.pass&racer=49&value=1" | check_success

# ... for a total of 38 heats
# 17/4 ~ 4 ratio for Lions & Tigers
# 13/4 ~ 3 ratio for Whites's Wolves
# 4/4 = 1 ratio for Bears & Freres
# 4/4 = 1 ratio for Webelos

# Schedule dens
curl_post action.php "action=schedule.generate&roundid=1" | check_success
curl_post action.php "action=schedule.generate&roundid=2" | check_success
curl_post action.php "action=schedule.generate&roundid=3" | check_success
curl_post action.php "action=schedule.generate&roundid=4" | check_success
# Can't schedule Arrows, because no one's checked in
curl_post action.php "action=schedule.generate&roundid=5" | check_failure

curl_getj "action.php?query=json.poll.coordinator" | \
    jq ".[\"current-heat\"] | .[\"now_racing\"] == false and .roundid == 1" | \
    expect_eq true

curl_postj action.php "action=json.heat.select&heat=next-up&now_racing=0" | check_jsuccess
curl_postj action.php "action=json.heat.select&now_racing=1" | check_jsuccess

curl_getj "action.php?query=json.poll.coordinator" | \
    jq ".[\"current-heat\"] | .[\"now_racing\"] == true and .roundid == 1 and .heat == 1" | \
    expect_eq true

## This script generated from the output of:
## timer/testing/fake-timer -t -l 4 localhost/xsite

user_login_timer
curl_post action.php "action=timer-message&message=HELLO" | check_success
curl_post action.php "action=timer-message&message=IDENTIFIED&nlanes=4" | check_success
cat $DEBUG_CURL | expect_one "<heat-ready[ />]"

run_heat 1  1 2.6158 3.7824 2.0463 2.0036
run_heat 1  2 2.1873 2.7640 3.2498 2.7619
run_heat 1  3 3.5283 3.2649 2.3900 2.1653
run_heat 1  4 3.6831 3.9044 3.2217 3.2740
run_heat 2  1 3.9820 2.3631 2.6738 3.0597
run_heat 2  2 2.5833 3.5019 2.9375 3.7031
run_heat 2  3 3.7282 2.7925 2.9317 3.4691
run_heat 3  1 3.6444 -      -      2.4671
run_heat 4  1 2.8700 -      2.2015 2.8728
run_heat 1  5 2.9534 2.2606 3.6678 2.6781
run_heat 1  6 3.8819 3.3501 3.7373 3.1624
run_heat 1  7 3.2352 2.7528 3.6898 2.7172
run_heat 1  8 2.0539 3.8099 3.8300 2.5309
run_heat 2  4 3.5253 3.0825 2.0197 2.0079
run_heat 2  5 3.2461 2.3070 2.7460 3.1656
run_heat 2  6 3.9903 2.1272 2.9032 2.3380
run_heat 3  2 3.4030 3.9654 -      -
run_heat 4  2 2.5979 3.8489 -      2.7025
run_heat 1  9 3.5098 2.9721 2.7074 2.6940
run_heat 1 10 3.4633 2.2940 3.8348 2.1791
run_heat 1 11 2.4415 2.3792 3.8497 3.1925
run_heat 1 12 3.2020 3.7824 2.0173 3.4068
run_heat 2  7 2.7497 2.1385 3.6122 2.5128
run_heat 2  8 3.2886 3.5359 3.6490 2.4214
run_heat 2  9 2.4732 3.8659 3.7021 2.7243
run_heat 3  3 -      3.9649 3.7225 -
run_heat 4  3 3.3670 3.0116 2.9083 -
run_heat 1 13 2.6151 2.8070 2.3636 3.6158
run_heat 1 14 2.0063 3.7848 3.6190 2.4874
run_heat 1 15 2.0059 3.5972 3.8646 3.9890
run_heat 1 16 2.5804 3.2428 2.0773 2.5551
run_heat 2 10 3.4255 3.4075 3.0036 3.3740
run_heat 2 11 3.3379 3.6183 3.2268 2.7605
run_heat 2 12 3.0604 3.1527 2.5206 2.0743
run_heat 3  4 -      -      2.2914 2.1275
run_heat 4  4 -      3.7456 3.1559 2.8862
run_heat 1 17 3.0536 2.1045 2.2103 3.5851
run_heat 2 13 3.9767 3.6456 2.2947 3.7196    x
# Expecting NO heat-ready:
cat $DEBUG_CURL | expect_count "<heat-ready[ />]" 0

curl_getj "action.php?query=json.poll.coordinator" | \
    jq ".[\"current-heat\"] | .[\"now_racing\"] == false" | \
    expect_eq true

user_login_coordinator
# Now create a aggregate round, 3 from each den.
# (Bears/Freres have only 2 racers and Webelos only 3.)
curl_postj action.php "action=json.roster.new&roundid=&top=3&bucketed=1&roundid_1=1&roundid_2=1&roundid_3=1&roundid_4=1&classname=Grand%20Finals" \
 | check_jsuccess

curl_getj "action.php?query=json.poll.coordinator" | \
    jq ".[\"current-heat\"] | .[\"now_racing\"] == false and .roundid == 2 and .heat == 13" | \
    expect_eq true

curl_post action.php "action=schedule.generate&roundid=8" | check_success

curl_getj "action.php?query=json.poll.coordinator" | \
    jq ".[\"current-heat\"] | .[\"now_racing\"] == false and .roundid == 8 and .heat == 1" | \
    expect_eq true

curl_postj action.php "action=json.heat.select&heat=next-up&now_racing=0" | check_jsuccess
curl_postj action.php "action=json.heat.select&now_racing=1" | check_jsuccess

run_heat 8 1  3 2 1 4

# Next heat is Grand Finals heat 2
curl_getj "action.php?query=json.poll.coordinator" | \
    jq ".[\"current-heat\"] | .[\"now_racing\"] == true and .roundid == 8 and .heat == 2" | \
    expect_eq true

curl_get "action.php?query=class.list" | expect_one 'Grand Finals'

# Unschedule and remove Grand Finals round
curl_post action.php "action=result.delete&roundid=8&heat=1" | check_success
curl_post action.php "action=schedule.unschedule&roundid=8" | check_success
curl_postj action.php "action=json.roster.delete&roundid=8" | check_jsuccess

curl_get "action.php?query=class.list" | expect_count 'Grand Finals' 0
curl_get "action.php?query=class.list" | expect_count 'classid=.8.' 0
