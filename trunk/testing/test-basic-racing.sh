#! /bin/sh

BASE_URL=$1
source common.sh

./login-coordinator.sh $BASE_URL

### Check in every other racer...
curl_post action.php "action=pass&racer=1&value=1" | check_success
curl_post action.php "action=pass&racer=3&value=1" | check_success
curl_post action.php "action=pass&racer=5&value=1" | check_success
curl_post action.php "action=pass&racer=7&value=1" | check_success
curl_post action.php "action=pass&racer=9&value=1" | check_success
curl_post action.php "action=pass&racer=11&value=1" | check_success
curl_post action.php "action=pass&racer=13&value=1" | check_success
curl_post action.php "action=pass&racer=15&value=1" | check_success
curl_post action.php "action=pass&racer=17&value=1" | check_success
curl_post action.php "action=pass&racer=19&value=1" | check_success
curl_post action.php "action=pass&racer=21&value=1" | check_success
curl_post action.php "action=pass&racer=23&value=1" | check_success
curl_post action.php "action=pass&racer=25&value=1" | check_success
curl_post action.php "action=pass&racer=27&value=1" | check_success
curl_post action.php "action=pass&racer=29&value=1" | check_success
curl_post action.php "action=pass&racer=31&value=1" | check_success
curl_post action.php "action=pass&racer=33&value=1" | check_success
curl_post action.php "action=pass&racer=35&value=1" | check_success
curl_post action.php "action=pass&racer=37&value=1" | check_success
curl_post action.php "action=pass&racer=39&value=1" | check_success
curl_post action.php "action=pass&racer=41&value=1" | check_success
curl_post action.php "action=pass&racer=43&value=1" | check_success
curl_post action.php "action=pass&racer=45&value=1" | check_success
curl_post action.php "action=pass&racer=47&value=1" | check_success
curl_post action.php "action=pass&racer=49&value=1" | check_success

### Schedule round 1 for 3 classes
curl_post action.php "action=schedule&roundid=1" | check_success
curl_post action.php "action=schedule&roundid=2" | check_success
curl_post action.php "action=schedule&roundid=3" | check_success

curl_post action.php "action=select-heat&roundid=1&now_racing=1" | check_success

curl_post action.php "action=timer-message&message=HELLO&nlanes=4" | check_success

### Racing for roundid=1: 5 heats
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


curl_post action.php "action=select-heat&roundid=2&now_racing=1" | check_success
curl_post action.php "action=timer-message&message=HEARTBEAT" | check_success

### Racing for roundid=2: 5 heats
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
curl_post action.php "action=pass&racer=13&value=0" | check_success
curl_post action.php "action=pass&racer=23&value=0" | check_success
curl_post action.php "action=pass&racer=33&value=0" | check_success
curl_post action.php "action=schedule&roundid=3" | check_success

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
