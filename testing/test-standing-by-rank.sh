#! /bin/bash

BASE_URL=$1
set -e -E -o pipefail
source `dirname $0`/common.sh

user_login_coordinator

`dirname $0`/reset-database.sh "$BASE_URL"

curl_post action.php "action=settings.write&do-use-subgroups=1&do-use-subgroups-checkbox=1" | check_success

curl_post action.php "action=racer.import&firstname=Jewell&lastname=Jeansonne&classname=ThePack&carnumber=101&subgroup=Lions" | check_success
curl_post action.php "action=racer.import&firstname=Cedrick&lastname=Charley&classname=ThePack&carnumber=202&subgroup=Tigers" | check_success
curl_post action.php "action=racer.import&firstname=Grant&lastname=Gribble&classname=ThePack&carnumber=303&subgroup=Bears" | check_success
curl_post action.php "action=racer.import&firstname=Harrison&lastname=Hanks&classname=ThePack&carnumber=404&subgroup=Wolves" | check_success
curl_post action.php "action=racer.import&firstname=Jerald&lastname=Jerry&classname=ThePack&carnumber=105&subgroup=Lions" | check_success
curl_post action.php "action=racer.import&firstname=Ernest&lastname=Edelman&classname=ThePack&carnumber=206&subgroup=Tigers" | check_success
curl_post action.php "action=racer.import&firstname=Lloyd&lastname=Lightsey&classname=ThePack&carnumber=307&subgroup=Bears" | check_success
curl_post action.php "action=racer.import&firstname=Gary&lastname=Grissom&classname=ThePack&carnumber=408&subgroup=Wolves" | check_success
curl_post action.php "action=racer.import&firstname=Von&lastname=Vassar&classname=ThePack&carnumber=109&subgroup=Lions" | check_success
curl_post action.php "action=racer.import&firstname=Harold&lastname=Hayek&classname=ThePack&carnumber=210&subgroup=Tigers" | check_success
curl_post action.php "action=racer.import&firstname=Pasquale&lastname=Procopio&classname=ThePack&carnumber=311&subgroup=Bears" | check_success
curl_post action.php "action=racer.import&firstname=Winford&lastname=Weld&classname=ThePack&carnumber=412&subgroup=Wolves" | check_success
curl_post action.php "action=racer.import&firstname=John&lastname=Jefferys&classname=ThePack&carnumber=113&subgroup=Lions" | check_success
curl_post action.php "action=racer.import&firstname=Thanh&lastname=Turner&classname=ThePack&carnumber=214&subgroup=Tigers" | check_success
curl_post action.php "action=racer.import&firstname=Demetrius&lastname=Demming&classname=ThePack&carnumber=315&subgroup=Bears" | check_success
curl_post action.php "action=racer.import&firstname=Everette&lastname=Esses&classname=ThePack&carnumber=416&subgroup=Wolves" | check_success
curl_post action.php "action=racer.import&firstname=Forrest&lastname=Figgins&classname=ThePack&carnumber=117&subgroup=Lions" | check_success
curl_post action.php "action=racer.import&firstname=Sterling&lastname=Spalla&classname=ThePack&carnumber=218&subgroup=Tigers" | check_success
curl_post action.php "action=racer.import&firstname=Delmar&lastname=Donnelly&classname=ThePack&carnumber=319&subgroup=Bears" | check_success
curl_post action.php "action=racer.import&firstname=Stanton&lastname=Salmon&classname=ThePack&carnumber=420&subgroup=Wolves" | check_success

curl_post action.php "action=racer.pass&value=1&racer=1" | check_success
curl_post action.php "action=racer.pass&value=1&racer=2" | check_success
curl_post action.php "action=racer.pass&value=1&racer=3" | check_success
curl_post action.php "action=racer.pass&value=1&racer=4" | check_success
curl_post action.php "action=racer.pass&value=1&racer=5" | check_success
curl_post action.php "action=racer.pass&value=1&racer=6" | check_success
curl_post action.php "action=racer.pass&value=1&racer=7" | check_success
curl_post action.php "action=racer.pass&value=1&racer=8" | check_success
curl_post action.php "action=racer.pass&value=1&racer=9" | check_success
curl_post action.php "action=racer.pass&value=1&racer=10" | check_success
curl_post action.php "action=racer.pass&value=1&racer=11" | check_success
curl_post action.php "action=racer.pass&value=1&racer=12" | check_success
curl_post action.php "action=racer.pass&value=1&racer=13" | check_success
curl_post action.php "action=racer.pass&value=1&racer=14" | check_success
curl_post action.php "action=racer.pass&value=1&racer=15" | check_success
curl_post action.php "action=racer.pass&value=1&racer=16" | check_success
curl_post action.php "action=racer.pass&value=1&racer=17" | check_success
curl_post action.php "action=racer.pass&value=1&racer=18" | check_success
curl_post action.php "action=racer.pass&value=1&racer=19" | check_success
curl_post action.php "action=racer.pass&value=1&racer=20" | check_success

curl_post action.php "action=settings.write&n-lanes=4" | check_success
curl_post action.php "action=schedule.generate&roundid=1" | check_success

curl_post action.php "action=select-heat&roundid=1&now_racing=1" | check_success

# Fast Lions    1.100
# Slow Tigers   5.500
# Mixed Bears, Wolves 2.00, 2.50, 3.00

# Racing chart (1=Lions, etc.):

user_login_timer
curl_post action.php "action=timer-message&message=HELLO" | check_success
curl_post action.php "action=timer-message&message=IDENTIFIED&nlanes=4" | check_success

check_heat_ready && curl_post action.php "action=timer-message&message=STARTED" | check_success
curl_post action.php "action=timer-message&message=FINISHED&lane1=1.01&lane2=2.503&lane3=1.17&lane4=2.508" | check_success
check_heat_ready && curl_post action.php "action=timer-message&message=STARTED" | check_success
curl_post action.php "action=timer-message&message=FINISHED&lane1=5.502&lane2=2.504&lane3=5.518&lane4=1.09" | check_success
check_heat_ready && curl_post action.php "action=timer-message&message=STARTED" | check_success
curl_post action.php "action=timer-message&message=FINISHED&lane1=2.503&lane2=1.05&lane3=2.519&lane4=5.510" | check_success
check_heat_ready && curl_post action.php "action=timer-message&message=STARTED" | check_success
curl_post action.php "action=timer-message&message=FINISHED&lane1=2.504&lane2=5.506&lane3=3.020&lane4=3.011" | check_success
check_heat_ready && curl_post action.php "action=timer-message&message=STARTED" | check_success
curl_post action.php "action=timer-message&message=FINISHED&lane1=1.05&lane2=2.507&lane3=1.01&lane4=3.012" | check_success
check_heat_ready && curl_post action.php "action=timer-message&message=STARTED" | check_success
curl_post action.php "action=timer-message&message=FINISHED&lane1=1.13&lane2=3.015&lane3=1.09&lane4=3.020" | check_success
check_heat_ready && curl_post action.php "action=timer-message&message=STARTED" | check_success
curl_post action.php "action=timer-message&message=FINISHED&lane1=3.012&lane2=5.514&lane3=2.508&lane4=2.519" | check_success
check_heat_ready && curl_post action.php "action=timer-message&message=STARTED" | check_success
curl_post action.php "action=timer-message&message=FINISHED&lane1=3.011&lane2=1.13&lane3=2.507&lane4=5.518" | check_success
check_heat_ready && curl_post action.php "action=timer-message&message=STARTED" | check_success
curl_post action.php "action=timer-message&message=FINISHED&lane1=5.514&lane2=3.016&lane3=5.510&lane4=1.01" | check_success
check_heat_ready && curl_post action.php "action=timer-message&message=STARTED" | check_success
curl_post action.php "action=timer-message&message=FINISHED&lane1=3.015&lane2=1.17&lane3=3.011&lane4=5.502" | check_success
check_heat_ready && curl_post action.php "action=timer-message&message=STARTED" | check_success
curl_post action.php "action=timer-message&message=FINISHED&lane1=3.016&lane2=5.518&lane3=3.012&lane4=2.503" | check_success
check_heat_ready && curl_post action.php "action=timer-message&message=STARTED" | check_success
curl_post action.php "action=timer-message&message=FINISHED&lane1=5.506&lane2=2.508&lane3=5.502&lane4=1.13" | check_success
check_heat_ready && curl_post action.php "action=timer-message&message=STARTED" | check_success
curl_post action.php "action=timer-message&message=FINISHED&lane1=2.507&lane2=1.09&lane3=2.503&lane4=5.514" | check_success
check_heat_ready && curl_post action.php "action=timer-message&message=STARTED" | check_success
curl_post action.php "action=timer-message&message=FINISHED&lane1=2.508&lane2=5.510&lane3=2.504&lane4=3.015" | check_success
check_heat_ready && curl_post action.php "action=timer-message&message=STARTED" | check_success
curl_post action.php "action=timer-message&message=FINISHED&lane1=1.09&lane2=3.011&lane3=1.05&lane4=3.016" | check_success
check_heat_ready && curl_post action.php "action=timer-message&message=STARTED" | check_success
curl_post action.php "action=timer-message&message=FINISHED&lane1=5.510&lane2=3.012&lane3=5.506&lane4=1.17" | check_success
check_heat_ready && curl_post action.php "action=timer-message&message=STARTED" | check_success
curl_post action.php "action=timer-message&message=FINISHED&lane1=5.518&lane2=3.020&lane3=5.514&lane4=1.05" | check_success
check_heat_ready && curl_post action.php "action=timer-message&message=STARTED" | check_success
curl_post action.php "action=timer-message&message=FINISHED&lane1=1.17&lane2=2.519&lane3=1.13&lane4=2.504" | check_success
check_heat_ready && curl_post action.php "action=timer-message&message=STARTED" | check_success
curl_post action.php "action=timer-message&message=FINISHED&lane1=3.020&lane2=5.502&lane3=3.016&lane4=2.507" | check_success
check_heat_ready && curl_post action.php "action=timer-message&message=STARTED" | check_success
curl_post action.php "action=timer-message&message=FINISHED&lane1=2.519&lane2=1.01&lane3=3.015&lane4=5.506" | check_success

user_login_coordinator

curl_get "standings.php" | grep '<tr' | expect_count "data-rankid=.2." 5
# curl_get would parse the HTML and leave one tag per line
curl_text "standings.php" | grep '<tr' | grep "data-rankid=.2." | grep "insubgroup.>4<" | expect_one "Thanh Turner"

curl_get "action.php?query=standings.reveal" | expect_count round 0
curl_post action.php "action=standings.reveal&roundid=" | check_success
curl_get "action.php?query=standings.reveal" | expect_one '<round roundid=.. rankid=..>Pack</round>'
curl_post action.php "action=standings.reveal&roundid=1" | check_success
curl_get "action.php?query=standings.reveal" | expect_one '<round roundid=.1. rankid=..>ThePack</round>'
curl_post action.php "action=standings.reveal&roundid=1&rankid=2" | check_success
curl_get "action.php?query=standings.reveal" | expect_one '<round roundid=.1. rankid=.2.>Tigers</round>'

# Rank-specific awards:

curl_post action.php "action=award.import&awardname=Most%20Lionic&awardtype=Design%20General&subgroup=Lions" | check_success
curl_post action.php "action=award.import&awardname=Most%20Tigerific&awardtype=Other&subgroup=Tigers" | check_success
curl_post action.php "action=award.import&awardname=The%20Pack%20Design&awardtype=Design%20Trophy&classname=ThePack" | check_success
curl_post action.php "action=award.import&awardname=Bad%20Subgroup%20Name&awardtype=Design%20Trophy&subgroup=Typo" | check_failure

curl_post action.php "action=award.winner&awardid=1&racerid=2" | expect_failure
curl_post action.php "action=award.winner&awardid=1&racerid=1" | expect_success
