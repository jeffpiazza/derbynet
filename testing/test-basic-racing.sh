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

### Racing for roundid=1: 5 heats
curl_postj action.php "action=heat.select&roundid=1&now_racing=1" | check_jsuccess

user_login_timer
curl_post action.php "action=timer-message&message=HELLO" | check_success
curl_post action.php "action=timer-message&message=IDENTIFIED&nlanes=4" | check_success

staged_heat4 101 121 141 111
run_heat 1 1 3.3294 3.4179 3.8182 2.2401
staged_heat4 111 131 101 121
run_heat 1 2 3.7554 2.6205 2.3800 3.2347
staged_heat4 121 141 111 131
run_heat 1 3 2.0793 3.6770 2.9511 2.8799
staged_heat4 131 101 121 141
run_heat 1 4 3.7412 3.4053 3.3414 2.8045
staged_heat4 141 111 131 101
run_heat 1 5 2.9661 3.9673 3.5686 3.8388    x

### Racing for roundid=2: 5 heats
user_login_coordinator
curl_postj action.php "action=heat.select&roundid=2&now_racing=1" | check_jsuccess

user_login_timer
curl_post action.php "action=timer-message&message=HEARTBEAT" | check_success
cat $DEBUG_CURL | expect_one "<heat-ready[ />]"

run_heat 2 1 2.6149 2.0731 3.0402 3.7937
run_heat 2 2 2.9945 3.4571 2.1867 2.3447

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

run_heat 2 3 2.4901 2.0838 3.6469 2.1003
run_heat 2 4 3.9403 3.4869 3.5717 3.5386
run_heat 2 5 3.0439 3.4090 3.3881 2.9110      x

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

run_heat 3 1 2.7706 -      2.4508 2.4413
run_heat 3 2 2.7384 3.0321 -      9.9999
run_heat 3 3 9.9999 3.4490 2.8584 -
run_heat 3 4 -      9.9999 2.7565 3.0264   x

user_login_coordinator
### Editing racers

[ `curl_get checkin.php | grep '"racerid":5,' | grep -c '"class":"Arrows'` -eq 1 ] || test_fails Initial class
curl_postj action.php "action=racer.edit&racer=5&firstname=Zuzu&lastname=Zingelo&carno=999&carname=Z-Car&rankid=4" | check_jsuccess
[ `curl_get checkin.php | grep '"racerid":5,' | grep -c '"firstname":"Zuzu"'` -eq 1 ] || test_fails Firstname change
[ `curl_get checkin.php | grep '"racerid":5,' | grep -c '"lastname":"Zingelo"'` -eq 1 ] || test_fails Lastname change
[ `curl_get checkin.php | grep '"racerid":5,' | grep -c '"class":"Webelos'` -eq 1 ] || test_fails Class change
[ `curl_get checkin.php | grep '"racerid":5,' | grep -c '"carnumber":999'` -eq 1 ] || test_fails Car number change

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

run_heat 4 1 3.5819 3.3400 2.7394 3.9177
run_heat 4 2 3.4442 2.4411 3.6294 2.4309
run_heat 4 3 3.4408 3.6778 2.9768 3.7326
run_heat 4 4 3.4894 2.0477 2.4732 2.5479
run_heat 4 5 2.4360 3.4858 2.7608 3.4163
run_heat 4 6 3.6268 2.5541 2.5365 3.9059
run_heat 4 7 2.5955 3.7658 3.7152 3.4789
run_heat 4 8 3.6487 3.0060 3.9589 3.6175    x

### Racing for roundid=5
user_login_coordinator
curl_postj action.php "action=heat.select&roundid=5&now_racing=1" | check_jsuccess
user_login_timer
curl_post action.php "action=timer-message&message=HEARTBEAT" | check_success

run_heat 5 1 3.9962 3.9847 2.1091 3.2685
run_heat 5 2 2.4600 3.6349 2.3152 2.6711
run_heat 5 3 3.8841 2.8243 2.7381 3.9018
run_heat 5 4 2.7886 3.5121 3.8979 2.0171   x

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

