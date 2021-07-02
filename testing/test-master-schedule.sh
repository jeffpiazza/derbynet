#! /bin/bash

BASE_URL=$1
set -e -E -o pipefail
source `dirname $0`/common.sh

user_login_coordinator

curl_postj action.php "action=settings.write&use-master-sched-checkbox=1&use-master-sched=1" | check_jsuccess
curl_postj action.php "action=settings.write&n-lanes=4" | check_jsuccess

# Two racers from Bears & Freres (but 4 heats, with byes)
curl_postj action.php "action=racer.pass&racer=3&value=1" | check_jsuccess
curl_postj action.php "action=racer.pass&racer=8&value=1" | check_jsuccess

# All the Lions and Tigers (17)
curl_postj action.php "action=racer.pass&racer=1&value=1" | check_jsuccess
curl_postj action.php "action=racer.pass&racer=6&value=1" | check_jsuccess
curl_postj action.php "action=racer.pass&racer=11&value=1" | check_jsuccess
curl_postj action.php "action=racer.pass&racer=16&value=1" | check_jsuccess
curl_postj action.php "action=racer.pass&racer=21&value=1" | check_jsuccess
curl_postj action.php "action=racer.pass&racer=26&value=1" | check_jsuccess
curl_postj action.php "action=racer.pass&racer=31&value=1" | check_jsuccess
curl_postj action.php "action=racer.pass&racer=36&value=1" | check_jsuccess
curl_postj action.php "action=racer.pass&racer=41&value=1" | check_jsuccess
curl_postj action.php "action=racer.pass&racer=46&value=1" | check_jsuccess
curl_postj action.php "action=racer.pass&racer=51&value=1" | check_jsuccess
curl_postj action.php "action=racer.pass&racer=56&value=1" | check_jsuccess
curl_postj action.php "action=racer.pass&racer=61&value=1" | check_jsuccess
curl_postj action.php "action=racer.pass&racer=71&value=1" | check_jsuccess
curl_postj action.php "action=racer.pass&racer=66&value=1" | check_jsuccess
curl_postj action.php "action=racer.pass&racer=76&value=1" | check_jsuccess
curl_postj action.php "action=racer.pass&racer=81&value=1" | check_jsuccess

# White's Wolves (13)
curl_postj action.php "action=racer.pass&racer=2&value=1" | check_jsuccess
curl_postj action.php "action=racer.pass&racer=7&value=1" | check_jsuccess
curl_postj action.php "action=racer.pass&racer=12&value=1" | check_jsuccess
curl_postj action.php "action=racer.pass&racer=22&value=1" | check_jsuccess
curl_postj action.php "action=racer.pass&racer=17&value=1" | check_jsuccess
curl_postj action.php "action=racer.pass&racer=27&value=1" | check_jsuccess
curl_postj action.php "action=racer.pass&racer=32&value=1" | check_jsuccess
curl_postj action.php "action=racer.pass&racer=37&value=1" | check_jsuccess
curl_postj action.php "action=racer.pass&racer=42&value=1" | check_jsuccess
curl_postj action.php "action=racer.pass&racer=52&value=1" | check_jsuccess
curl_postj action.php "action=racer.pass&racer=62&value=1" | check_jsuccess
curl_postj action.php "action=racer.pass&racer=72&value=1" | check_jsuccess
curl_postj action.php "action=racer.pass&racer=82&value=1" | check_jsuccess

# Three racers from Webelos (4 heats, with byes)
curl_postj action.php "action=racer.pass&racer=39&value=1" | check_jsuccess
curl_postj action.php "action=racer.pass&racer=44&value=1" | check_jsuccess
curl_postj action.php "action=racer.pass&racer=49&value=1" | check_jsuccess

# ... for a total of 38 heats
# 17/4 ~ 4 ratio for Lions & Tigers
# 13/4 ~ 3 ratio for Whites's Wolves
# 4/4 = 1 ratio for Bears & Freres
# 4/4 = 1 ratio for Webelos

# Schedule dens
curl_postj action.php "action=schedule.generate&roundid=1" | check_jsuccess
curl_postj action.php "action=schedule.generate&roundid=2" | check_jsuccess
curl_postj action.php "action=schedule.generate&roundid=3" | check_jsuccess
curl_postj action.php "action=schedule.generate&roundid=4" | check_jsuccess
# Can't schedule Arrows, because no one's checked in
curl_postj action.php "action=schedule.generate&roundid=5" | check_jfailure

curl_getj "action.php?query=poll.coordinator" | \
    jq ".[\"current-heat\"] | .[\"now_racing\"] == false and .roundid == 1" | \
    expect_eq true

curl_postj action.php "action=heat.select&heat=next-up&now_racing=0" | check_jsuccess
curl_postj action.php "action=heat.select&now_racing=1" | check_jsuccess

curl_getj "action.php?query=poll.coordinator" | \
    jq ".[\"current-heat\"] | .[\"now_racing\"] == true and .roundid == 1 and .heat == 1" | \
    expect_eq true

## This script generated from the output of:
## timer/testing/fake-timer -t -l 4 localhost/xsite

user_login_timer
curl_post action.php "action=timer-message&message=HELLO" | check_success
curl_post action.php "action=timer-message&message=IDENTIFIED&nlanes=4" | check_success
cat $DEBUG_CURL | expect_one "<heat-ready[ />]"

run_heat 1 1   101:2.6158 111:3.7824 126:2.0463 146:2.0036
run_heat 1 2   106:2.1873 116:2.7640 131:3.2498 151:2.7619
run_heat 1 3   111:3.5283 121:3.2649 136:2.3900 156:2.1653
run_heat 1 4   116:3.6831 126:3.9044 141:3.2217 161:3.2740
run_heat 2 1   202:3.9820 212:2.3631 232:2.6738 227:3.0597
run_heat 2 2   207:2.5833 217:3.5019 237:2.9375 232:3.7031
run_heat 2 3   212:3.7282 222:2.7925 242:2.9317 237:3.4691
run_heat 3 1   303:3.6444 -          -          308:2.4671
run_heat 4 1   508:2.8700 -          510:2.2015 509:2.8728
run_heat 1 5   121:2.9534 131:2.2606 146:3.6678 166:2.6781
run_heat 1 6   126:3.8819 136:3.3501 151:3.7373 171:3.1624
run_heat 1 7   131:3.2352 141:2.7528 156:3.6898 176:2.7172
run_heat 1 8   136:2.0539 146:3.8099 161:3.8300 181:2.5309
run_heat 2 4   217:3.5253 227:3.0825 252:2.0197 242:2.0079
run_heat 2 5   222:3.2461 232:2.3070 262:2.7460 252:3.1656
run_heat 2 6   227:3.9903 237:2.1272 272:2.9032 262:2.3380
run_heat 3 2   308:3.4030 303:3.9654 -          -
run_heat 4 2   509:2.5979 508:3.8489 -          510:2.7025
run_heat 1 9   141:3.5098 151:2.9721 166:2.7074 101:2.6940
run_heat 1 10  146:3.4633 156:2.2940 171:3.8348 106:2.1791
run_heat 1 11  151:2.4415 161:2.3792 176:3.8497 111:3.1925
run_heat 1 12  156:3.2020 166:3.7824 181:2.0173 116:3.4068
run_heat 2 7   232:2.7497 242:2.1385 282:3.6122 272:2.5128
run_heat 2 8   237:3.2886 252:3.5359 202:3.6490 282:2.4214
run_heat 2 9   242:2.4732 262:3.8659 207:3.7021 202:2.7243
run_heat 3 3   -          308:3.9649 303:3.7225 -
run_heat 4 3   510:3.3670 509:3.0116 508:2.9083 -
run_heat 1 13  161:2.6151 171:2.8070 101:2.3636 121:3.6158
run_heat 1 14  166:2.0063 176:3.7848 106:3.6190 126:2.4874
run_heat 1 15  171:2.0059 181:3.5972 111:3.8646 131:3.9890
run_heat 1 16  176:2.5804 101:3.2428 116:2.0773 136:2.5551
run_heat 2 10  252:3.4255 272:3.4075 212:3.0036 207:3.3740
run_heat 2 11  262:3.3379 282:3.6183 217:3.2268 212:2.7605
run_heat 2 12  272:3.0604 202:3.1527 222:2.5206 217:2.0743
run_heat 3 4   -          -          308:2.2914 303:2.1275
run_heat 4 4   -          510:3.7456 509:3.1559 508:2.8862
run_heat 1 17  181:3.0536 106:2.1045 121:2.2103 141:3.5851
run_heat 2 13  282:3.9767 207:3.6456 227:2.2947 222:3.7196  x

curl_getj "action.php?query=poll.coordinator" | \
    jq ".[\"current-heat\"] | .[\"now_racing\"] == false" | \
    expect_eq true

user_login_coordinator
# Now create a aggregate round, 3 from each den.
# (Bears/Freres have only 2 racers and Webelos only 3.)
curl_postj action.php "action=roster.new&roundid=&top=3&bucketed=1&classid_1=1&classid_2=1&classid_3=1&classid_4=1&classname=Grand%20Finals" \
    | check_jsuccess
jq -e '.finalists | map(.racerid) | sort == [1,3,6,8,32,36,37,39,42,44,49]' $DEBUG_CURL >/dev/null || test_fails

curl_getj "action.php?query=poll.coordinator" | \
    jq ".[\"current-heat\"] | .[\"now_racing\"] == false and .roundid == 2 and .heat == 13" | \
    expect_eq true

curl_postj action.php "action=schedule.generate&roundid=8" | check_jsuccess

curl_getj "action.php?query=poll.coordinator" | \
    jq ".[\"current-heat\"] | .[\"now_racing\"] == false and .roundid == 8 and .heat == 1" | \
    expect_eq true

curl_postj action.php "action=heat.select&heat=next-up&now_racing=0" | check_jsuccess
curl_postj action.php "action=heat.select&now_racing=1" | check_jsuccess


run_heat 8 1  106:3 101:2 232:1 508:4

# Next heat is Grand Finals heat 2
curl_getj "action.php?query=poll.coordinator" | \
    jq ".[\"current-heat\"] | .[\"now_racing\"] == true and .roundid == 8 and .heat == 2" | \
    expect_eq true

curl_getj "action.php?query=class.list" | expect_one 'Grand Finals'

# Unschedule and remove Grand Finals round
curl_postj action.php "action=result.delete&roundid=8&heat=1" | check_jsuccess
curl_postj action.php "action=schedule.unschedule&roundid=8" | check_jsuccess
curl_postj action.php "action=roster.delete&roundid=8" | check_jsuccess

curl_getj "action.php?query=class.list" | expect_count 'Grand Finals' 0
curl_getj "action.php?query=class.list" | \
    jq '.classes | all(.classid != 8)' | expect_eq true
