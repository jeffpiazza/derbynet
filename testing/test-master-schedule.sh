#! /bin/bash

BASE_URL=$1
set -e -E -o pipefail
source `dirname $0`/common.sh

`dirname $0`/login-coordinator.sh $BASE_URL

curl_post action.php "action=settings.write&use-master-sched-checkbox=1&use-master-sched=1" | check_success
curl_post action.php "action=settings.write&n-lanes=4" | check_success

## Check in just two racers from one den, and everyone from another den.
## What happens if only two dens get scheduled when racing starts?
## What happens if click on the "Racing" button when no heat is current?
## What happens if more dens get scheduled after racing starts?
## What happens if more dens get scheduled after the first few have raced?
## What does the "Next to Race" button actually do?  What action does it send?
## What happens if only two lanes report for a four-lane heat?

# Two racers from Bears & Freres
curl_post action.php "action=pass&racer=3&value=1" | check_success
curl_post action.php "action=pass&racer=8&value=1" | check_success

# All the Lions and Tigers
curl_post action.php "action=pass&racer=1&value=1" | check_success
curl_post action.php "action=pass&racer=6&value=1" | check_success
curl_post action.php "action=pass&racer=11&value=1" | check_success
curl_post action.php "action=pass&racer=16&value=1" | check_success
curl_post action.php "action=pass&racer=21&value=1" | check_success
curl_post action.php "action=pass&racer=26&value=1" | check_success
curl_post action.php "action=pass&racer=31&value=1" | check_success
curl_post action.php "action=pass&racer=36&value=1" | check_success
curl_post action.php "action=pass&racer=41&value=1" | check_success
curl_post action.php "action=pass&racer=46&value=1" | check_success
curl_post action.php "action=pass&racer=51&value=1" | check_success
curl_post action.php "action=pass&racer=56&value=1" | check_success
curl_post action.php "action=pass&racer=61&value=1" | check_success
curl_post action.php "action=pass&racer=71&value=1" | check_success
curl_post action.php "action=pass&racer=66&value=1" | check_success
curl_post action.php "action=pass&racer=76&value=1" | check_success
curl_post action.php "action=pass&racer=81&value=1" | check_success

# White's Wolves
curl_post action.php "action=pass&racer=2&value=1" | check_success
curl_post action.php "action=pass&racer=7&value=1" | check_success
curl_post action.php "action=pass&racer=12&value=1" | check_success
curl_post action.php "action=pass&racer=22&value=1" | check_success
curl_post action.php "action=pass&racer=17&value=1" | check_success
curl_post action.php "action=pass&racer=27&value=1" | check_success
curl_post action.php "action=pass&racer=32&value=1" | check_success
curl_post action.php "action=pass&racer=37&value=1" | check_success
curl_post action.php "action=pass&racer=42&value=1" | check_success
curl_post action.php "action=pass&racer=52&value=1" | check_success
curl_post action.php "action=pass&racer=62&value=1" | check_success
curl_post action.php "action=pass&racer=72&value=1" | check_success
curl_post action.php "action=pass&racer=82&value=1" | check_success

# Three racers from Webelos
curl_post action.php "action=pass&racer=39&value=1" | check_success
curl_post action.php "action=pass&racer=44&value=1" | check_success
curl_post action.php "action=pass&racer=49&value=1" | check_success

# Schedule dens
curl_post action.php "action=schedule&roundid=1" | check_success
curl_post action.php "action=schedule&roundid=2" | check_success
curl_post action.php "action=schedule&roundid=3" | check_success
curl_post action.php "action=schedule&roundid=4" | check_success
# Can't schedule Arrows, because no one's checked in
curl_post action.php "action=schedule&roundid=5" | check_failure

curl_post action.php "action=select-heat&heat=first&now_racing=0" | check_success
curl_post action.php "action=select-heat&now_racing=1" | check_success

## This script generated from the output of:
## timer/testing/fake-timer -t -l 4 localhost/xsite

curl_post action.php "action=timer-message&message=HELLO" | check_success
curl_post action.php "action=timer-message&message=IDENTIFIED&nlanes=4" | check_success
check_heat_ready && curl_post action.php "action=timer-message&message=STARTED" | check_success
curl_post action.php "action=timer-message&message=FINISHED&lane1=2.6158383929791826&lane2=3.7824151491468916&lane3=2.046356489153065&lane4=2.003682139082848" | check_success
check_heat_ready && curl_post action.php "action=timer-message&message=STARTED" | check_success
curl_post action.php "action=timer-message&message=FINISHED&lane1=2.1873838224800712&lane2=2.764048974941116&lane3=3.249892321495438&lane4=2.761923231133686" | check_success
check_heat_ready && curl_post action.php "action=timer-message&message=STARTED" | check_success
curl_post action.php "action=timer-message&message=FINISHED&lane1=3.5283085301534527&lane2=3.264980719020013&lane3=2.390058419265598&lane4=2.1653326303272076" | check_success
check_heat_ready && curl_post action.php "action=timer-message&message=STARTED" | check_success
curl_post action.php "action=timer-message&message=FINISHED&lane1=3.683192054700733&lane2=3.9044453918132067&lane3=3.2217523707528946&lane4=3.274054247923223" | check_success
check_heat_ready && curl_post action.php "action=timer-message&message=STARTED" | check_success
curl_post action.php "action=timer-message&message=FINISHED&lane1=3.9820101740449725&lane2=2.363113258131479&lane3=2.673884250399599&lane4=3.059748785244528" | check_success
check_heat_ready && curl_post action.php "action=timer-message&message=STARTED" | check_success
curl_post action.php "action=timer-message&message=FINISHED&lane1=2.5833783181652556&lane2=3.5019314130034687&lane3=2.9375278593499523&lane4=3.703109016991307" | check_success
check_heat_ready && curl_post action.php "action=timer-message&message=STARTED" | check_success
curl_post action.php "action=timer-message&message=FINISHED&lane1=3.7282309893320607&lane2=2.7925520157065904&lane3=2.9317743035821917&lane4=3.4691054355629696" | check_success
check_heat_ready && curl_post action.php "action=timer-message&message=STARTED" | check_success
curl_post action.php "action=timer-message&message=FINISHED&lane1=3.6444276383263774&lane4=2.4671224840941495" | check_success
check_heat_ready && curl_post action.php "action=timer-message&message=STARTED" | check_success
curl_post action.php "action=timer-message&message=FINISHED&lane1=2.87000590064682&lane3=2.2015270618105856&lane4=2.872854054192925" | check_success
check_heat_ready && curl_post action.php "action=timer-message&message=STARTED" | check_success
curl_post action.php "action=timer-message&message=FINISHED&lane1=2.953417257150347&lane2=2.2606351546851555&lane3=3.667835392136949&lane4=2.6781661414899" | check_success
check_heat_ready && curl_post action.php "action=timer-message&message=STARTED" | check_success
curl_post action.php "action=timer-message&message=FINISHED&lane1=3.8819926451042495&lane2=3.3501830270726236&lane3=3.737302621011842&lane4=3.1624010855401847" | check_success
check_heat_ready && curl_post action.php "action=timer-message&message=STARTED" | check_success
curl_post action.php "action=timer-message&message=FINISHED&lane1=3.235250661181368&lane2=2.7528974730940945&lane3=3.6898587199980772&lane4=2.7172596408851053" | check_success
check_heat_ready && curl_post action.php "action=timer-message&message=STARTED" | check_success
curl_post action.php "action=timer-message&message=FINISHED&lane1=2.0539994411678477&lane2=3.809942786548026&lane3=3.8300246262081563&lane4=2.530939849375616" | check_success
check_heat_ready && curl_post action.php "action=timer-message&message=STARTED" | check_success
curl_post action.php "action=timer-message&message=FINISHED&lane1=3.5253027178602245&lane2=3.0825166740232017&lane3=2.0197619464294174&lane4=2.007983011583347" | check_success
check_heat_ready && curl_post action.php "action=timer-message&message=STARTED" | check_success
curl_post action.php "action=timer-message&message=FINISHED&lane1=3.246111972639162&lane2=2.307082231946798&lane3=2.7460233669789416&lane4=3.165668317383176" | check_success
check_heat_ready && curl_post action.php "action=timer-message&message=STARTED" | check_success
curl_post action.php "action=timer-message&message=FINISHED&lane1=3.9903797864190858&lane2=2.1272280459360626&lane3=2.9032399894925005&lane4=2.338072457172167" | check_success
check_heat_ready && curl_post action.php "action=timer-message&message=STARTED" | check_success
curl_post action.php "action=timer-message&message=FINISHED&lane1=3.4030549111693285&lane2=3.965499222101794" | check_success
check_heat_ready && curl_post action.php "action=timer-message&message=STARTED" | check_success
curl_post action.php "action=timer-message&message=FINISHED&lane1=2.597935963616326&lane2=3.8489881383289113&lane4=2.702563062017679" | check_success
check_heat_ready && curl_post action.php "action=timer-message&message=STARTED" | check_success
curl_post action.php "action=timer-message&message=FINISHED&lane1=3.509869571704267&lane2=2.9721133336661967&lane3=2.707473046763745&lane4=2.694080779308644" | check_success
check_heat_ready && curl_post action.php "action=timer-message&message=STARTED" | check_success
curl_post action.php "action=timer-message&message=FINISHED&lane1=3.4633535331949923&lane2=2.2940032823263907&lane3=3.834879791670719&lane4=2.1791123338498757" | check_success
check_heat_ready && curl_post action.php "action=timer-message&message=STARTED" | check_success
curl_post action.php "action=timer-message&message=FINISHED&lane1=2.4415845072956337&lane2=2.379233016509463&lane3=3.849794772634443&lane4=3.1925831860688154" | check_success
check_heat_ready && curl_post action.php "action=timer-message&message=STARTED" | check_success
curl_post action.php "action=timer-message&message=FINISHED&lane1=3.2020142503800364&lane2=3.7824109350283965&lane3=2.017392215463988&lane4=3.4068665900108037" | check_success
check_heat_ready && curl_post action.php "action=timer-message&message=STARTED" | check_success
curl_post action.php "action=timer-message&message=FINISHED&lane1=2.7497164150831725&lane2=2.1385855208733964&lane3=3.6122885384987686&lane4=2.512863003074563" | check_success
check_heat_ready && curl_post action.php "action=timer-message&message=STARTED" | check_success
curl_post action.php "action=timer-message&message=FINISHED&lane1=3.288605350692154&lane2=3.5359865067061844&lane3=3.6490586948207904&lane4=2.421454426054618" | check_success
check_heat_ready && curl_post action.php "action=timer-message&message=STARTED" | check_success
curl_post action.php "action=timer-message&message=FINISHED&lane1=2.473208849677036&lane2=3.865910568708282&lane3=3.7021557608482016&lane4=2.724360844291385" | check_success
check_heat_ready && curl_post action.php "action=timer-message&message=STARTED" | check_success
curl_post action.php "action=timer-message&message=FINISHED&lane2=3.9649628560647256&lane3=3.722565905774613" | check_success
check_heat_ready && curl_post action.php "action=timer-message&message=STARTED" | check_success
curl_post action.php "action=timer-message&message=FINISHED&lane1=3.367052652374851&lane2=3.01162623814701&lane3=2.908322765899624" | check_success
check_heat_ready && curl_post action.php "action=timer-message&message=STARTED" | check_success
curl_post action.php "action=timer-message&message=FINISHED&lane1=2.6151441643108075&lane2=2.807002799643116&lane3=2.363667550836766&lane4=3.6158154526062294" | check_success
check_heat_ready && curl_post action.php "action=timer-message&message=STARTED" | check_success
curl_post action.php "action=timer-message&message=FINISHED&lane1=2.0063410549210396&lane2=3.784894724678603&lane3=3.619021262050724&lane4=2.4874835040793717" | check_success
check_heat_ready && curl_post action.php "action=timer-message&message=STARTED" | check_success
curl_post action.php "action=timer-message&message=FINISHED&lane1=2.005999152900232&lane2=3.597254064700995&lane3=3.8646256713326004&lane4=3.9890011651952735" | check_success
check_heat_ready && curl_post action.php "action=timer-message&message=STARTED" | check_success
curl_post action.php "action=timer-message&message=FINISHED&lane1=3.4255016241696&lane2=3.4075242444439247&lane3=3.003681483551642&lane4=3.374070437184562" | check_success
check_heat_ready && curl_post action.php "action=timer-message&message=STARTED" | check_success
curl_post action.php "action=timer-message&message=FINISHED&lane1=3.337996044700041&lane2=3.6183351183773222&lane3=3.2268339820328307&lane4=2.760532832913643" | check_success
check_heat_ready && curl_post action.php "action=timer-message&message=STARTED" | check_success
curl_post action.php "action=timer-message&message=FINISHED&lane3=2.2914831577387527&lane4=2.127547510930656" | check_success
check_heat_ready && curl_post action.php "action=timer-message&message=STARTED" | check_success
curl_post action.php "action=timer-message&message=FINISHED&lane2=3.7456253772555783&lane3=3.1559613286877757&lane4=2.886256564128961" | check_success
check_heat_ready && curl_post action.php "action=timer-message&message=STARTED" | check_success
curl_post action.php "action=timer-message&message=FINISHED&lane1=2.5804390397370502&lane2=3.242868201096835&lane3=2.0773245124390782&lane4=2.555133983515451" | check_success
check_heat_ready && curl_post action.php "action=timer-message&message=STARTED" | check_success
curl_post action.php "action=timer-message&message=FINISHED&lane1=3.060475327644565&lane2=3.152793275845924&lane3=2.520643874734715&lane4=2.0743723911558085" | check_success
check_heat_ready && curl_post action.php "action=timer-message&message=STARTED" | check_success
curl_post action.php "action=timer-message&message=FINISHED&lane1=3.053688339388506&lane2=2.1045298369344447&lane3=2.2103917486327904&lane4=3.5851139868733126" | check_success
check_heat_ready && curl_post action.php "action=timer-message&message=STARTED" | check_success
curl_post action.php "action=timer-message&message=FINISHED&lane1=3.9767622641546687&lane2=3.6456777886682827&lane3=2.2947077461067202&lane4=3.719661658730179" | check_success
