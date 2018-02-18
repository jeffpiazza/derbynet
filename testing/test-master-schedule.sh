#! /bin/bash

BASE_URL=$1
set -e -E -o pipefail
source `dirname $0`/common.sh

user_login_coordinator

curl_post action.php "action=settings.write&use-master-sched-checkbox=1&use-master-sched=1" | check_success
curl_post action.php "action=settings.write&n-lanes=4" | check_success

## What does the "Next to Race" button actually do?  What action does it send?

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

curl_post action.php "action=select-heat&heat=next-up&now_racing=0" | check_success
curl_post action.php "action=select-heat&now_racing=1" | check_success

## This script generated from the output of:
## timer/testing/fake-timer -t -l 4 localhost/xsite

user_login_timer
curl_post action.php "action=timer-message&message=HELLO" | check_success
curl_post action.php "action=timer-message&message=IDENTIFIED&nlanes=4" | check_success
cat $DEBUG_CURL | expect_one "<heat-ready[ />]"

run_heat 1  1 2.6158383929791826 3.7824151491468916 2.046356489153065  2.003682139082848
run_heat 1  2 2.1873838224800712 2.764048974941116  3.249892321495438  2.761923231133686
run_heat 1  3 3.5283085301534527 3.264980719020013  2.390058419265598  2.1653326303272076
run_heat 1  4 3.683192054700733  3.9044453918132067 3.2217523707528946 3.274054247923223
run_heat 2  1 3.9820101740449725 2.363113258131479  2.673884250399599  3.059748785244528
run_heat 2  2 2.5833783181652556 3.5019314130034687 2.9375278593499523 3.703109016991307
run_heat 2  3 3.7282309893320607 2.7925520157065904 2.9317743035821917 3.4691054355629696
run_heat 3  1 3.6444276383263774 -                  -                  2.4671224840941495
run_heat 4  1 2.87000590064682   -                  2.2015270618105856 2.872854054192925
run_heat 1  5 2.953417257150347  2.2606351546851555 3.667835392136949  2.6781661414899
run_heat 1  6 3.8819926451042495 3.3501830270726236 3.737302621011842  3.1624010855401847
run_heat 1  7 3.235250661181368  2.7528974730940945 3.6898587199980772 2.7172596408851053
run_heat 1  8 2.0539994411678477 3.809942786548026  3.8300246262081563 2.530939849375616
run_heat 2  4 3.5253027178602245 3.0825166740232017 2.0197619464294174 2.007983011583347
run_heat 2  5 3.246111972639162  2.307082231946798  2.7460233669789416 3.165668317383176
run_heat 2  6 3.9903797864190858 2.1272280459360626 2.9032399894925005 2.338072457172167
run_heat 3  2 3.4030549111693285 3.965499222101794  -                  -
run_heat 4  2 2.597935963616326  3.8489881383289113 -                  2.702563062017679
run_heat 1  9 3.509869571704267  2.9721133336661967 2.707473046763745  2.694080779308644
run_heat 1 10 3.4633535331949923 2.2940032823263907 3.834879791670719  2.1791123338498757
run_heat 1 11 2.4415845072956337 2.379233016509463  3.849794772634443  3.1925831860688154
run_heat 1 12 3.2020142503800364 3.7824109350283965 2.017392215463988  3.4068665900108037
run_heat 2  7 2.7497164150831725 2.1385855208733964 3.6122885384987686 2.512863003074563
run_heat 2  8 3.288605350692154  3.5359865067061844 3.6490586948207904 2.421454426054618
run_heat 2  9 2.473208849677036  3.865910568708282  3.7021557608482016 2.724360844291385
run_heat 3  3 -                  3.9649628560647256 3.722565905774613  -
run_heat 4  3 3.367052652374851  3.01162623814701   2.908322765899624  -
run_heat 1 13 2.6151441643108075 2.807002799643116  2.363667550836766  3.6158154526062294
run_heat 1 14 2.0063410549210396 3.784894724678603  3.619021262050724  2.4874835040793717
run_heat 1 15 2.005999152900232  3.597254064700995  3.8646256713326004 3.9890011651952735
run_heat 1 16 2.5804390397370502 3.242868201096835  2.0773245124390782 2.555133983515451
run_heat 2 10 3.4255016241696    3.4075242444439247 3.003681483551642  3.374070437184562
run_heat 2 11 3.337996044700041  3.6183351183773222 3.2268339820328307 2.760532832913643
run_heat 2 12 3.060475327644565  3.152793275845924  2.520643874734715  2.0743723911558085
run_heat 3  4 -                  -                  2.2914831577387527 2.127547510930656
run_heat 4  4 -                  3.7456253772555783 3.1559613286877757 2.886256564128961
run_heat 1 17 3.053688339388506  2.1045298369344447 2.2103917486327904 3.5851139868733126
run_heat 2 13 3.9767622641546687 3.6456777886682827 2.2947077461067202 3.719661658730179    x
# Expecting NO heat-ready:
cat $DEBUG_CURL | expect_count "<heat-ready[ />]" 0

curl_get "action.php?query=poll.coordinator" | grep current-heat | expect_one 'now-racing="0"'

user_login_coordinator
# Now create a Grand Finals round, 3 from each den.
# (Bears/Freres have only 2 racers and Webelos only 3.)
curl_post action.php "action=roster.new&roundid=&top=3&bucketed=1&roundid_1=1&roundid_2=1&roundid_3=1&roundid_4=1" \
 | check_success

curl_get "action.php?query=poll.coordinator" | grep current-heat | expect_one 'now-racing="0"'
curl_get "action.php?query=poll.coordinator" | grep current-heat | expect_one 'roundid="2"'
curl_get "action.php?query=poll.coordinator" | grep current-heat | expect_one 'heat="13"'

curl_post action.php "action=schedule.generate&roundid=8" | check_success

curl_get "action.php?query=poll.coordinator" | grep current-heat | expect_one 'now-racing="0"'
curl_get "action.php?query=poll.coordinator" | grep current-heat | expect_one 'roundid="2"'
curl_get "action.php?query=poll.coordinator" | grep current-heat | expect_one 'heat="13"'

curl_post action.php "action=select-heat&heat=next-up&now_racing=0" | check_success
curl_post action.php "action=select-heat&now_racing=1" | check_success

run_heat 8 1  3 2 1 4

# Next heat is Grand Finals heat 2
curl_get "action.php?query=poll.coordinator" | grep current-heat | expect_one 'roundid="8"'
curl_get "action.php?query=poll.coordinator" | grep current-heat | expect_one 'heat="2"'
curl_get "action.php?query=poll.coordinator" | grep current-heat | expect_one 'now-racing="1"'

# Unschedule and remove Grand Finals round
curl_post action.php "action=result.delete&roundid=8&heat=1" | check_success
curl_post action.php "action=schedule.unschedule&roundid=8" | check_success
curl_post action.php "action=roster.delete&roundid=8" | check_success
