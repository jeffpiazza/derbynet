#! /bin/bash

BASE_URL=$1
set -e -E -o pipefail
source `dirname $0`/common.sh

user_login_coordinator

curl_postj action.php "action=settings.write&unused-lane-mask=0&n-lanes=4" | check_jsuccess

### Check in every other racer...
`dirname $0`/test-basic-checkins.sh "$BASE_URL"

### Schedule first round for 3 of the classes
curl_postj action.php "action=schedule.generate&roundid=1" | check_jsuccess
curl_postj action.php "action=schedule.generate&roundid=2" | check_jsuccess
curl_postj action.php "action=schedule.generate&roundid=3" | check_jsuccess

# Can't delete a racer who's in a schedule
curl_postj action.php "action=racer.delete&racer=21" | check_jfailure

# This is testing replay connected before we start racing
## curl_postj action.php "action=replay.message" | jq -e ".replay == [\"HELLO\"]" \
##        >/dev/null || test_fails

### Racing for roundid=1: 5 heats
curl_postj action.php "action=heat.select&roundid=1&now_racing=1" | check_jsuccess

user_login_timer
curl_post action.php "action=timer-message&message=HELLO" | check_success
curl_post action.php "action=timer-message&message=IDENTIFIED&nlanes=4" | check_success

# TODO test replay connects after "now racing" started.

run_heat 1 1 101:3.3294 121:3.4179 141:3.8182 111:2.2401

curl_postj action.php "action=replay.message" | \
    jq -e '.replay == ["CANCEL", "START Lions & Tigers_Round1_Heat01",
                       "RACE_STARTS 4 2 0.5",
                       "REPLAY 4 2 0.5",
                       "CANCEL",
                       "START Lions & Tigers_Round1_Heat02"]' \
       >/dev/null || test_fails

run_heat 1 2 111:3.7554 131:2.6205 101:2.3800 121:3.2347
run_heat 1 3 121:2.0793 141:3.6770 111:2.9511 131:2.8799
run_heat 1 4 131:3.7412 101:3.4053 121:3.3414 141:2.8045
run_heat 1 5 141:2.9661 111:3.9673 131:3.5686 101:3.8388    x

### Racing for roundid=2: 5 heats
user_login_coordinator
curl_postj action.php "action=heat.select&roundid=2&now_racing=1" | check_jsuccess

user_login_timer
curl_post action.php "action=timer-message&message=HEARTBEAT" | check_success
cat $DEBUG_CURL | expect_one "<heat-ready[ />]"

run_heat 2 1  207:2.6149 227:2.0731 247:3.0402 217:3.7937
run_heat 2 2  217:2.9945 237:3.4571 207:2.1867 227:2.3447

user_login_coordinator
curl_getj "action.php?query=poll.coordinator" | jq '.["last-heat"] == "available"' | expect_eq true
curl_postj action.php "action=heat.rerun&heat=last" | check_jsuccess
curl_getj "action.php?query=poll.coordinator" | \
    jq '.["last-heat"] == "recoverable" and 
        (.["heat-results"] | all(has("finishtime") and has("finishplace")))' | \
    expect_eq true

curl_postj action.php "action=heat.reinstate" | grep last-heat | expect_one none
curl_getj "action.php?query=poll.coordinator" | \
    jq '.racers |
        all((.finishtime == 2.994 and (.name | test("Darrell.*"))) or 
            (.finishtime == 3.457 and .name == "Ian Ives") or
            (.finishtime == 2.187 and .name == "Blake Burling") or
            (.finishtime == 2.345 and .name == "Elliot Eastman"))' | \
    expect_eq true

curl_postj action.php "action=heat.select&heat=next&now_racing=1" | check_jsuccess
user_login_timer

run_heat 2 3  227:2.4901 247:2.0838 217:3.6469 237:2.1003
run_heat 2 4  237:3.9403 207:3.4869 227:3.5717 247:3.5386
run_heat 2 5  247:3.0439 217:3.4090 237:3.3881 207:2.9110  x
 
user_login_coordinator
### Un-checkin a few roundid=3 and re-generate schedule
curl_postj action.php "action=racer.pass&racer=13&value=0" | check_jsuccess
curl_postj action.php "action=racer.pass&racer=23&value=0" | check_jsuccess
curl_postj action.php "action=racer.pass&racer=33&value=0" | check_jsuccess
curl_postj action.php "action=schedule.generate&roundid=3" | check_jsuccess

curl_postj action.php "action=heat.select&roundid=3&now_racing=1" | check_jsuccess

user_login_timer
curl_post action.php "action=timer-message&message=HEARTBEAT" | check_success

### Racing for roundid=3: 4 heats among 3 racers

run_heat 3 1  303:2.7706 -          343:2.4508 139:2.4413
run_heat 3 2  139:2.7384 303:3.0321 -          343:9.9999
run_heat 3 3  343:9.9999 139:3.4490 303:2.8584 -
run_heat 3 4  -          343:9.9999 139:2.7565 303:3.0264  x

user_login_coordinator
### Editing racers

[ `curl_get checkin.php | grep '"racerid":5,' | grep -c '"class":"Arrows'` -eq 1 ] || test_fails Initial class
curl_postj action.php "action=racer.edit&racer=5&firstname=Zuzu&lastname=Zingelo&carno=999&carname=Z-Car&rankid=4&note_from=Suburbia" | check_jsuccess
## To construct the checkin table, checkin.php includes a bunch of javascript
## addRow0 calls manufactured by PHP code.
[ `curl_get checkin.php | grep '"racerid":5,' | grep -c '"firstname":"Zuzu"'` -eq 1 ] || test_fails Firstname change
[ `curl_get checkin.php | grep '"racerid":5,' | grep -c '"lastname":"Zingelo"'` -eq 1 ] || test_fails Lastname change
[ `curl_get checkin.php | grep '"racerid":5,' | grep -c '"class":"Webelos'` -eq 1 ] || test_fails Class change
[ `curl_get checkin.php | grep '"racerid":5,' | grep -c '"carnumber":999'` -eq 1 ] || test_fails Car number change
[ `curl_get checkin.php | grep '"racerid":5,' | grep -c '"note":"Suburbia"'` -eq 1 ] || test_fails note_from change

curl_get checkin.php | expect_count '"note":null' 0
# Not sure how many, but the pattern is:
#   curl_get checkin.php | expect_count '"note":""' 10


### Overwriting manual heat results: Clobber Dereck Dreier's results to all be 8.888
curl_postj action.php "action=heat.select&roundid=1&heat=1&now_racing=0" | check_jsuccess
curl_postj action.php "action=result.write&lane2=8.888" | check_jsuccess
curl_postj action.php "action=heat.select&roundid=1&heat=2" | check_jsuccess
curl_postj action.php "action=result.write&lane4=8.888" | check_jsuccess
curl_postj action.php "action=heat.select&roundid=1&heat=3" | check_jsuccess
curl_postj action.php "action=result.write&lane1=8.888" | check_jsuccess
curl_postj action.php "action=heat.select&roundid=1&heat=4" | check_jsuccess
curl_postj action.php "action=result.write&lane3=8.888" | check_jsuccess

# For roundid 4, schedule two appearances per lane per racer
curl_postj action.php "action=schedule.generate&roundid=4&n_times_per_lane=2" | check_jsuccess
# Schedule for roundid 5
curl_postj action.php "action=schedule.generate&roundid=5" | check_jsuccess

### Racing for roundid=4
curl_postj action.php "action=heat.select&roundid=4&now_racing=1" | check_jsuccess
user_login_timer
curl_post action.php "action=timer-message&message=HEARTBEAT" | check_success

run_heat 4 1  999:3.5819 510:3.3400 502:2.7394 null:3.9177
run_heat 4 2  502:3.4442 999:2.4411 510:3.6294 null:2.4309
run_heat 4 3  510:3.4408 502:3.6778 999:2.9768 null:3.7326
run_heat 4 4  510:3.4894 502:2.0477 999:2.4732 null:2.5479
run_heat 4 5  999:2.4360 502:3.4858 510:2.7608 null:3.4163
run_heat 4 6  502:3.6268 510:2.5541 999:2.5365 null:3.9059
run_heat 4 7  510:2.5955 999:3.7658 502:3.7152 null:3.4789
run_heat 4 8  999:3.6487 502:3.0060 510:3.9589 null:3.6175  x

### Racing for roundid=5
user_login_coordinator
curl_postj action.php "action=heat.select&roundid=5&now_racing=1" | check_jsuccess
user_login_timer
curl_post action.php "action=timer-message&message=HEARTBEAT" | check_success

run_heat 5 1  415:3.9962 445:3.9847 435:2.1091 425:3.2685
run_heat 5 2  425:2.4600 415:3.6349 445:2.3152 435:2.6711
run_heat 5 3  435:3.8841 425:2.8243 415:2.7381 445:3.9018
run_heat 5 4  445:2.7886 435:3.5121 425:3.8979 415:2.0171  x

user_login_coordinator

curl_postj action.php "action=award.present&key=speed-1" | check_jsuccess
curl_getj "action.php?query=award.current" | expect_one Elliot
curl_postj action.php "action=award.present&key=speed-2" | check_jsuccess
curl_getj "action.php?query=award.current" | expect_one Kris
curl_postj action.php "action=award.present&key=speed-3" | check_jsuccess
curl_getj "action.php?query=award.current" | expect_one Blake

curl_postj action.php "action=award.present&key=speed-1-4" | check_jsuccess
curl_getj "action.php?query=award.current" | expect_one Kaba
curl_postj action.php "action=award.present&key=speed-2-4" | check_jsuccess
curl_getj "action.php?query=award.current" | expect_one Zuzu
curl_postj action.php "action=award.present&key=speed-3-4" | check_jsuccess
curl_getj "action.php?query=award.current" | expect_one Byron

# Make sure that excluding Carroll Cybulski leaves Adolpho Asher as the second-in-tigers winner
curl_postj action.php "action=award.present&key=speed-2-1" | check_jsuccess
curl_getj "action.php?query=award.current" | expect_one Asher

# Issue#87: make sure the awards presentation page populates for speed awards
curl_get awards-presentation.php | expect_one "<option data-classid=.1.>Lions &amp; Tigers</option>"
curl_get awards-presentation.php | expect_count ">3rd Fastest in " 6

curl_postj action.php "action=settings.write&one-trophy-per=1&one-trophy-per-checkbox" | check_jsuccess
# There are only 3 racers in Webelos, and one of them gets a pack-level award.  So only
# five 3rd-place trophies in this case.
curl_get awards-presentation.php | expect_count ">3rd Fastest in " 5

curl_postj action.php "action=award.present&key=speed-1" | check_jsuccess
curl_getj "action.php?query=award.current" | expect_one Elliot
curl_postj action.php "action=award.present&key=speed-2" | check_jsuccess
curl_getj "action.php?query=award.current" | expect_one Kris
curl_postj action.php "action=award.present&key=speed-3" | check_jsuccess
curl_getj "action.php?query=award.current" | expect_one Blake

curl_postj action.php "action=award.present&key=speed-1-4" | check_jsuccess
curl_getj "action.php?query=award.current" | expect_one Zuzu
curl_postj action.php "action=award.present&key=speed-2-4" | check_jsuccess
curl_getj "action.php?query=award.current" | expect_one Byron

# Issue#97: confirm lane bias calculation isn't broken
curl_get history.php | expect_one "<td>1.757 sample variance.</td>"
curl_get history.php | expect_one "<td>1.867 sample variance.</td>"
curl_get history.php | expect_one "<td>1.832 sample variance.</td>"
curl_get history.php | expect_one "<td>1.966 sample variance.</td>"
curl_get history.php | expect_one "<p>F statistic is 0.090 with df1=3 and df2=85</p>"

