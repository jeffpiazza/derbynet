#! /bin/bash

BASE_URL=$1
set -e -E -o pipefail
source `dirname $0`/common.sh

user_login_coordinator

curl_post action.php "action=settings.write&n-lanes=4" | check_success

### Check in every other racer...
`dirname $0`/test-basic-checkins.sh "$BASE_URL"

### Schedule first round for 3 of the classes
curl_post action.php "action=schedule.generate&roundid=1" | check_success
curl_post action.php "action=schedule.generate&roundid=2" | check_success
curl_post action.php "action=schedule.generate&roundid=3" | check_success

### Racing for roundid=1: 5 heats
curl_post action.php "action=select-heat&roundid=1&now_racing=1" | check_success

user_login_timer
curl_post action.php "action=timer-message&message=HELLO" | check_success
curl_post action.php "action=timer-message&message=IDENTIFIED&nlanes=4" | check_success

staged_heat4 101 121 141 111
run_heat 1 1 3.3294268726240133 3.4179854414310484 3.818251820275865  2.240156635057904
staged_heat4 111 131 101 121
run_heat 1 2 3.755459366370065  2.6205858409266822 2.380029859598636  3.2347486741135887
staged_heat4 121 141 111 131
run_heat 1 3 2.079354701980928  3.6770361146010666 2.9511089893125515 2.8799167237857377
staged_heat4 131 101 121 141
run_heat 1 4 3.7412731387211764 3.405377024315312  3.341490063448355  2.804557990229122
staged_heat4 141 111 131 101
run_heat 1 5 2.96617950392236   3.9673731376083374 3.56865022781857   3.8388886921923415    x

### Racing for roundid=2: 5 heats
user_login_coordinator
curl_post action.php "action=select-heat&roundid=2&now_racing=1" | check_success

user_login_timer
curl_post action.php "action=timer-message&message=HEARTBEAT" | check_success
cat $DEBUG_CURL | expect_one "<heat-ready[ />]"

run_heat 2 1 2.614998506472289 2.073131117194488  3.0402460662858495 3.7937815988460155
run_heat 2 2 2.994596587596803 3.457181283243707  2.18676227631551   2.344727543260708
run_heat 2 3 2.490176080549383 2.083847477663076  3.646950464934686  2.1003158107733295
run_heat 2 4 3.940308674460656 3.4869952774053647 3.5717918276879654 3.538643938151412
run_heat 2 5 3.043976595882954 3.409014449582755  3.388110311263697  2.911083015110213      x

user_login_coordinator
### Un-checkin a few roundid=3 and re-generate schedule
curl_post action.php "action=racer.pass&racer=13&value=0" | check_success
curl_post action.php "action=racer.pass&racer=23&value=0" | check_success
curl_post action.php "action=racer.pass&racer=33&value=0" | check_success
curl_post action.php "action=schedule.generate&roundid=3" | check_success

curl_post action.php "action=select-heat&roundid=3&now_racing=1" | check_success

user_login_timer
curl_post action.php "action=timer-message&message=HEARTBEAT" | check_success

### Racing for roundid=3: 4 heats among 3 racers

run_heat 3 1 2.7706287330762995 -                  2.4508518847836402 2.441341067637084
run_heat 3 2 2.738401719134787  3.0321225923338653 -                  9.9999
run_heat 3 3 9.9999             3.4490285448987454 2.8584802815655985 -
run_heat 3 4 -                  9.9999             2.756576818870377  3.0264310036127933   x

user_login_coordinator
### Editing racers
[ `curl_get checkin.php | grep 'class-5"' | grep -c '>Arrows'` -eq 1 ] || test_fails Initial class
curl_post action.php "action=racer.edit&racer=5&firstname=Zuzu&lastname=Zingelo&carno=999&carname=Z-Car&rankid=4" | check_success
[ `curl_get checkin.php | grep firstname-5 | grep -c '>Zuzu</td>'` -eq 1 ] || test_fails Firstname change
[ `curl_get checkin.php | grep lastname-5 | grep -c '>Zingelo</td>'` -eq 1 ] || test_fails Lastname change
[ `curl_get checkin.php | grep 'class-5"' | grep -c '>Webelos'` -eq 1 ] || test_fails Class change
[ `curl_get checkin.php | grep car-number-5 | grep -c '>999</td>'` -eq 1 ] || test_fails Car number change

### Overwriting manual heat results: Clobber Dereck Dreier's results to all be 8.888
curl_post action.php "action=select-heat&roundid=1&heat=1&now_racing=0" | check_success
curl_post action.php "action=result.write&lane2=8.888" | check_success
curl_post action.php "action=select-heat&roundid=1&heat=2" | check_success
curl_post action.php "action=result.write&lane4=8.888" | check_success
curl_post action.php "action=select-heat&roundid=1&heat=3" | check_success
curl_post action.php "action=result.write&lane1=8.888" | check_success
curl_post action.php "action=select-heat&roundid=1&heat=4" | check_success
curl_post action.php "action=result.write&lane3=8.888" | check_success

# For roundid 4, schedule two appearances per lane per racer
curl_post action.php "action=schedule.generate&roundid=4&nrounds=2" | check_success
# Schedule for roundid 5
curl_post action.php "action=schedule.generate&roundid=5" | check_success

### Racing for roundid=4
curl_post action.php "action=select-heat&roundid=4&now_racing=1" | check_success
user_login_timer
curl_post action.php "action=timer-message&message=HEARTBEAT" | check_success

run_heat 4 1 3.581933364716356  3.3400731028218082 2.739493818706433  3.917729713673916
run_heat 4 2 3.4442507799131765 2.4411579340070277 3.6294505375852366 2.430947440406351
run_heat 4 3 3.4408039110826882 3.6778818405759583 2.976856611632552  3.7326024055828313
run_heat 4 4 3.48947787221569   2.047786539033757  2.4732761668871746 2.547997653720559
run_heat 4 5 2.4360085953891315 3.485896541487217  2.760802966619598  3.416321869253517
run_heat 4 6 3.626896364989139  2.5541112570192586 2.536503418909614  3.9059625894956485
run_heat 4 7 2.5955033883902923 3.7658907236550467 3.7152641900293784 3.4789389340166657
run_heat 4 8 3.648749488910531  3.0060133667294835 3.9589674579465677 3.617549894590156    x

### Racing for roundid=5
user_login_coordinator
curl_post action.php "action=select-heat&roundid=5&now_racing=1" | check_success
user_login_timer
curl_post action.php "action=timer-message&message=HEARTBEAT" | check_success

run_heat 5 1 3.9962279925545943 3.9847459663932026 2.1091805633909195 3.2685008030430156
run_heat 5 2 2.4600888959781884 3.634953062369908  2.315237412613288  2.671188533868289
run_heat 5 3 3.884104613683352  2.824318361529536  2.7381147675717172 3.9018944125092982
run_heat 5 4 2.788677259690628  3.5121727633843625 3.897900103278687  2.0171570935608143   x

user_login_coordinator
# Make sure that excluding Carroll Cybulski leaves Adolpho Asher as the second-in-tigers winner
curl_post action.php "action=award.present&key=speed-2-1" | check_success
curl_get "action.php?query=award.current" | expect_one Asher
