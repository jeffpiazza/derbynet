#! /bin/bash

BASE_URL=$1
set -e -E -o pipefail
source `dirname $0`/common.sh

`dirname $0`/login-coordinator.sh $BASE_URL

curl_post action.php "action=settings.write&n-lanes=4" | check_success

### Check in every other racer...
`dirname $0`/test-basic-checkins.sh "$BASE_URL"

### Schedule first round for 3 of the classes
curl_post action.php "action=schedule.generate&roundid=1" | check_success
curl_post action.php "action=schedule.generate&roundid=2" | check_success
curl_post action.php "action=schedule.generate&roundid=3" | check_success

### Racing for roundid=1: 5 heats
curl_post action.php "action=select-heat&roundid=1&now_racing=1" | check_success
curl_post action.php "action=timer-message&message=HELLO" | check_success
curl_post action.php "action=timer-message&message=IDENTIFIED&nlanes=4" | check_success
check_heat_ready && curl_post action.php "action=timer-message&message=STARTED" | check_success
curl_post action.php "action=timer-message&message=FINISHED&lane1=3.3294268726240133&lane2=3.4179854414310484&lane3=3.818251820275865&lane4=2.240156635057904" | check_success 
check_heat_ready && curl_post action.php "action=timer-message&message=STARTED" | check_success
curl_post action.php "action=timer-message&message=FINISHED&lane1=3.755459366370065&lane2=2.6205858409266822&lane3=2.380029859598636&lane4=3.2347486741135887" | check_success
check_heat_ready && curl_post action.php "action=timer-message&message=STARTED" | check_success
curl_post action.php "action=timer-message&message=FINISHED&lane1=2.079354701980928&lane2=3.6770361146010666&lane3=2.9511089893125515&lane4=2.8799167237857377" | check_success
check_heat_ready && curl_post action.php "action=timer-message&message=STARTED" | check_success
curl_post action.php "action=timer-message&message=FINISHED&lane1=3.7412731387211764&lane2=3.405377024315312&lane3=3.341490063448355&lane4=2.804557990229122" | check_success
check_heat_ready && curl_post action.php "action=timer-message&message=STARTED" | check_success
curl_post action.php "action=timer-message&message=FINISHED&lane1=2.96617950392236&lane2=3.9673731376083374&lane3=3.56865022781857&lane4=3.8388886921923415" | check_success

### Racing for roundid=2: 5 heats
curl_post action.php "action=select-heat&roundid=2&now_racing=1" | check_success
curl_post action.php "action=timer-message&message=HEARTBEAT" | check_success
check_heat_ready && curl_post action.php "action=timer-message&message=STARTED" | check_success
curl_post action.php "action=timer-message&message=FINISHED&lane1=2.614998506472289&lane2=2.073131117194488&lane3=3.0402460662858495&lane4=3.7937815988460155" | check_success
check_heat_ready && curl_post action.php "action=timer-message&message=STARTED" | check_success
curl_post action.php "action=timer-message&message=FINISHED&lane1=2.994596587596803&lane2=3.457181283243707&lane3=2.18676227631551&lane4=2.344727543260708" | check_success
check_heat_ready && curl_post action.php "action=timer-message&message=STARTED" | check_success
curl_post action.php "action=timer-message&message=FINISHED&lane1=2.490176080549383&lane2=2.083847477663076&lane3=3.646950464934686&lane4=2.1003158107733295" | check_success
check_heat_ready && curl_post action.php "action=timer-message&message=STARTED" | check_success
curl_post action.php "action=timer-message&message=FINISHED&lane1=3.940308674460656&lane2=3.4869952774053647&lane3=3.5717918276879654&lane4=3.538643938151412" | check_success
check_heat_ready && curl_post action.php "action=timer-message&message=STARTED" | check_success
curl_post action.php "action=timer-message&message=FINISHED&lane1=3.043976595882954&lane2=3.409014449582755&lane3=3.388110311263697&lane4=2.911083015110213" | check_success

### Un-checkin a few roundid=3 and re-generate schedule
curl_post action.php "action=racer.pass&racer=13&value=0" | check_success
curl_post action.php "action=racer.pass&racer=23&value=0" | check_success
curl_post action.php "action=racer.pass&racer=33&value=0" | check_success
curl_post action.php "action=schedule.generate&roundid=3" | check_success

curl_post action.php "action=select-heat&roundid=3&now_racing=1" | check_success
curl_post action.php "action=timer-message&message=HEARTBEAT" | check_success

### Racing for roundid=3: 4 heats among 2 racers
check_heat_ready && curl_post action.php "action=timer-message&message=STARTED" | check_success
## Bye lane times are just ignored
curl_post action.php "action=timer-message&message=FINISHED&lane1=2.7706287330762995&lane2=2.4761577976297398&lane3=2.4508518847836402&lane4=2.441341067637084" | check_success
check_heat_ready && curl_post action.php "action=timer-message&message=STARTED" | check_success
curl_post action.php "action=timer-message&message=FINISHED&lane1=2.738401719134787&lane2=3.0321225923338653" | check_success
check_heat_ready && curl_post action.php "action=timer-message&message=STARTED" | check_success
curl_post action.php "action=timer-message&message=FINISHED&lane2=3.4490285448987454&lane3=2.8584802815655985" | check_success
check_heat_ready && curl_post action.php "action=timer-message&message=STARTED" | check_success
curl_post action.php "action=timer-message&message=FINISHED&lane3=2.756576818870377&lane4=3.0264310036127933" | check_success

### Editing racers
[ `curl_get checkin.php | grep 'class-5"' | grep -c '>Arrows'` -eq 1 ] || test_fails Initial class
curl_post action.php "action=racer.edit&racer=5&firstname=Zuzu&lastname=Zingelo&carno=999&carname=Z-Car&rankid=4" | check_success
[ `curl_get checkin.php | grep firstname-5 | grep -c '>Zuzu</td>'` -eq 1 ] || test_fails Firstname change
[ `curl_get checkin.php | grep lastname-5 | grep -c '>Zingelo</td>'` -eq 1 ] || test_fails Lastname change
[ `curl_get checkin.php | grep 'class-5"' | grep -c '>Webelos'` -eq 1 ] || test_fails Class change
[ `curl_get checkin.php | grep car-number-5 | grep -c '>999</td>'` -eq 1 ] || test_fails Car number change

### Overwriting manual heat results: Clobber Dereck Dreier's results to all be 8.888
curl_post action.php "action=select-heat&roundid=1&heat=1&now_racing=0" | check_success
curl_post action.php "action=results.write&lane2=8.888" | check_success
curl_post action.php "action=select-heat&roundid=1&heat=2" | check_success
curl_post action.php "action=results.write&lane4=8.888" | check_success
curl_post action.php "action=select-heat&roundid=1&heat=3" | check_success
curl_post action.php "action=results.write&lane1=8.888" | check_success
curl_post action.php "action=select-heat&roundid=1&heat=4" | check_success
curl_post action.php "action=results.write&lane3=8.888" | check_success

# For roundid 4, schedule two appearances per lane per racer
curl_post action.php "action=schedule.generate&roundid=4&nrounds=2" | check_success
# Schedule for roundid 5
curl_post action.php "action=schedule.generate&roundid=5" | check_success

### Racing for roundid=4
curl_post action.php "action=select-heat&roundid=4&now_racing=1" | check_success
curl_post action.php "action=timer-message&message=HEARTBEAT" | check_success
check_heat_ready && curl_post action.php "action=timer-message&message=STARTED" | check_success
curl_post action.php "action=timer-message&message=FINISHED&lane1=3.581933364716356&lane2=3.3400731028218082&lane3=2.739493818706433&lane4=3.917729713673916" | check_success
check_heat_ready && curl_post action.php "action=timer-message&message=STARTED" | check_success
curl_post action.php "action=timer-message&message=FINISHED&lane1=3.4442507799131765&lane2=2.4411579340070277&lane3=3.6294505375852366&lane4=2.430947440406351" | check_success
check_heat_ready && curl_post action.php "action=timer-message&message=STARTED" | check_success
curl_post action.php "action=timer-message&message=FINISHED&lane1=3.4408039110826882&lane2=3.6778818405759583&lane3=2.976856611632552&lane4=3.7326024055828313" | check_success
check_heat_ready && curl_post action.php "action=timer-message&message=STARTED" | check_success
curl_post action.php "action=timer-message&message=FINISHED&lane1=3.48947787221569&lane2=2.047786539033757&lane3=2.4732761668871746&lane4=2.547997653720559" | check_success
check_heat_ready && curl_post action.php "action=timer-message&message=STARTED" | check_success
curl_post action.php "action=timer-message&message=FINISHED&lane1=2.4360085953891315&lane2=3.485896541487217&lane3=2.760802966619598&lane4=3.416321869253517" | check_success
check_heat_ready && curl_post action.php "action=timer-message&message=STARTED" | check_success
curl_post action.php "action=timer-message&message=FINISHED&lane1=3.626896364989139&lane2=2.5541112570192586&lane3=2.536503418909614&lane4=3.9059625894956485" | check_success
check_heat_ready && curl_post action.php "action=timer-message&message=STARTED" | check_success
curl_post action.php "action=timer-message&message=FINISHED&lane1=2.5955033883902923&lane2=3.7658907236550467&lane3=3.7152641900293784&lane4=3.4789389340166657" | check_success
check_heat_ready && curl_post action.php "action=timer-message&message=STARTED" | check_success
curl_post action.php "action=timer-message&message=FINISHED&lane1=3.648749488910531&lane2=3.0060133667294835&lane3=3.9589674579465677&lane4=3.617549894590156" | check_success

### Racing for roundid=5
curl_post action.php "action=select-heat&roundid=5&now_racing=1" | check_success
curl_post action.php "action=timer-message&message=HEARTBEAT" | check_success
check_heat_ready && curl_post action.php "action=timer-message&message=STARTED" | check_success
curl_post action.php "action=timer-message&message=FINISHED&lane1=3.9962279925545943&lane2=3.9847459663932026&lane3=2.1091805633909195&lane4=3.2685008030430156" | check_success
check_heat_ready && curl_post action.php "action=timer-message&message=STARTED" | check_success
curl_post action.php "action=timer-message&message=FINISHED&lane1=2.4600888959781884&lane2=3.634953062369908&lane3=2.315237412613288&lane4=2.671188533868289" | check_success
check_heat_ready && curl_post action.php "action=timer-message&message=STARTED" | check_success
curl_post action.php "action=timer-message&message=FINISHED&lane1=3.884104613683352&lane2=2.824318361529536&lane3=2.7381147675717172&lane4=3.9018944125092982" | check_success
check_heat_ready && curl_post action.php "action=timer-message&message=STARTED" | check_success
curl_post action.php "action=timer-message&message=FINISHED&lane1=2.788677259690628&lane2=3.5121727633843625&lane3=3.897900103278687&lane4=2.0171570935608143" | check_success

# Make sure that excluding Carroll Cybulski leaves Adolpho Asher as the second-in-tigers winner
curl_post action.php "action=award.present&key=speed-2-1" | check_success
curl_get "action.php?query=award.current" | expect_one Asher
