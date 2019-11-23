#! /bin/bash

BASE_URL=$1
set -e -E -o pipefail
source `dirname $0`/common.sh

user_login_coordinator

`dirname $0`/reset-database.sh "$BASE_URL"

curl_post action.php "action=settings.write&do-use-subgroups=1&do-use-subgroups-checkbox=1" | check_success

# Single group, ThePack
# Subgroup Lions = 1
# Subgroup Tigers = 2
# Subgroup Bears = 3
# Subgroup Wolves = 4

curl_post action.php "action=racer.import&firstname=Jewell&lastname=Jeansonne&classname=ThePack&carnumber=101&subgroup=Lions&exclude=" | check_success
curl_post action.php "action=racer.import&firstname=Cedrick&lastname=Charley&classname=ThePack&carnumber=202&subgroup=Tigers" | check_success
curl_post action.php "action=racer.import&firstname=Grant&lastname=Gribble&classname=ThePack&carnumber=303&subgroup=Bears" | check_success
curl_post action.php "action=racer.import&firstname=Harrison&lastname=Hanks&classname=ThePack&carnumber=404&subgroup=Wolves" | check_success
curl_post action.php "action=racer.import&firstname=Jerald&lastname=Jerry&classname=ThePack&carnumber=105&subgroup=Lions&exclude=X" | check_success
curl_post action.php "action=racer.import&firstname=Ernest&lastname=Edelman&classname=ThePack&carnumber=206&subgroup=Tigers" | check_success
curl_post action.php "action=racer.import&firstname=Lloyd&lastname=Lightsey&classname=ThePack&carnumber=307&subgroup=Bears" | check_success
curl_post action.php "action=racer.import&firstname=Gary&lastname=Grissom&classname=ThePack&carnumber=408&subgroup=Wolves" | check_success
curl_post action.php "action=racer.import&firstname=Von&lastname=Vassar&classname=ThePack&carnumber=109&subgroup=Lions&exclude=NO" | check_success
curl_post action.php "action=racer.import&firstname=Harold&lastname=Hayek&classname=ThePack&carnumber=210&subgroup=Tigers&exclude=yes" | check_success
curl_post action.php "action=racer.import&firstname=Pasquale&lastname=Procopio&classname=ThePack&carnumber=311&subgroup=Bears" | check_success
curl_post action.php "action=racer.import&firstname=Winford&lastname=Weld&classname=ThePack&carnumber=412&subgroup=Wolves" | check_success
curl_post action.php "action=racer.import&firstname=John&lastname=Jefferys&classname=ThePack&carnumber=113&subgroup=Lions&exclude=0" | check_success
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
cat $DEBUG_CURL | expect_one "<heat-ready[ />]"

run_heat 1  1 1.01  2.503 1.17  2.508
run_heat 1  2 5.502 2.504 5.518 1.09
run_heat 1  3 2.503 1.05  2.519 5.510
run_heat 1  4 2.504 5.506 3.020 3.011
run_heat 1  5 1.05  2.507 1.01  3.012
run_heat 1  6 1.13  3.015 1.09  3.020
run_heat 1  7 3.012 5.514 2.508 2.519
run_heat 1  8 3.011 1.13  2.507 5.518
run_heat 1  9 5.514 3.016 5.510 1.01
run_heat 1 10 3.015 1.17  3.011 5.502
run_heat 1 11 3.016 5.518 3.012 2.503
run_heat 1 12 5.506 2.508 5.502 1.13
run_heat 1 13 2.507 1.09  2.503 5.514
run_heat 1 14 2.508 5.510 2.504 3.015
run_heat 1 15 1.09  3.011 1.05  3.016
run_heat 1 16 5.510 3.012 5.506 1.17
run_heat 1 17 5.518 3.020 5.514 1.05
run_heat 1 18 1.17  2.519 1.13  2.504
run_heat 1 19 3.020 5.502 3.016 2.507
run_heat 1 20 2.519 1.01  3.015 5.506  x


# Standings by times:
# 101 Jewell Jeansonne
# 105 Jerald Jerry    -- ineligible
# 109 Von Vassar
# 113 John Jefferys
# 117 forrest Figgins
# 303 Grant Gribble
# 404 Harrison Hanks
# 307 Lloyd Lightsey
# 408 Gary Grissom
# 319 Delmar Donnelly
# 311 Pasquale Procopio
# 412 Winford Weld
# 315 Demetrius Demming
# 416 Everette Esses
# 420 Stanton Salmon
# 202 Cedrick Charley
# 206 Ernest Edelman
# 210 Harold Hayek  -- ineligible
# 214 Thanh Turner
# 218 Sterling Spalla

user_login_coordinator

curl_get "standings.php" | grep '<tr' | expect_count "data-rankid=.3." 5
# Because Harold is ineligible, only 4 show up
curl_get "standings.php" | grep '<tr' | expect_count "data-rankid=.2." 4

curl_text "export.php" | sed -n -e '/START_JSON/,/END_JSON/ p' | tail -2 | head -1 | \
    expect_one '[1,"101","Jewell Jeansonne","Lions",1,"4","1.010","1.010","1.010"]'
curl_text "export.php" | sed -n -e '/START_JSON/,/END_JSON/ p' | tail -2 | head -1 | \
    expect_one '[16,"206","Ernest Edelman","Tigers",2,"4","5.506","5.506","5.506"]'
curl_text "export.php" | sed -n -e '/START_JSON/,/END_JSON/ p' | tail -2 | head -1 | \
    expect_one '[17,"214","Thanh Turner","Tigers",3,"4","5.514","5.514","5.514"]'
curl_text "export.php" | sed -n -e '/START_JSON/,/END_JSON/ p' | tail -2 | head -1 | \
    expect_one '[18,"218","Sterling Spalla","Tigers",4,"4","5.518","5.518","5.518"]'

# curl_get would parse the HTML and leave one tag per line
# Thanh came in 4th by time, but with Harold ineligible, he becomes 3rd, and Sterling is 4th
curl_text "standings.php" | grep '<tr' | grep "data-rankid=.2." | grep "insubgroup.>4<" | expect_one "Sterling Spalla"

curl_get "action.php?query=standings.reveal" | expect_count catalog-entry 0
# javascript: encodeURIComponent(JSON.stringify({kind: 'supergroup', key: 'supergroup', name: 'Pack'}))
curl_post action.php "action=standings.reveal&catalog-entry=%7B%22kind%22%3A%22supergroup%22%2C%22key%22%3A%22supergroup%22%2C%22name%22%3A%22Pack%22%7D" | check_success
curl_get "action.php?query=standings.reveal" | expect_one '<catalog-entry json="{&quot;kind&quot;:&quot;supergroup&quot;,&quot;key&quot;:&quot;supergroup&quot;,&quot;name&quot;:&quot;Pack&quot;}"/>'

curl_post action.php "action=standings.reveal&catalog-entry=%7B%22kind%22%3A%22class%22%2C%22key%22%3A%22c1%22%2C%22name%22%3A%22ThePack%22%7D" | check_success
curl_get "action.php?query=standings.reveal" | expect_one '<catalog-entry json="{&quot;kind&quot;:&quot;class&quot;,&quot;key&quot;:&quot;c1&quot;,&quot;name&quot;:&quot;ThePack&quot;}"/>'
curl_post action.php "action=standings.reveal&catalog-entry=%7B%22kind%22%3A%22rank%22%2C%22key%22%3A%22r2%22%2C%22name%22%3A%22Tigers%22%7D" | check_success
curl_get "action.php?query=standings.reveal" | expect_one '<catalog-entry json="{&quot;kind&quot;:&quot;rank&quot;,&quot;key&quot;:&quot;r2&quot;,&quot;name&quot;:&quot;Tigers&quot;}"/>'

# Rank-specific awards:

curl_post action.php "action=award.import&awardname=Most%20Lionic&awardtype=Design%20General&subgroup=Lions" | check_success
curl_post action.php "action=award.import&awardname=Most%20Tigerific&awardtype=Other&subgroup=Tigers" | check_success
curl_post action.php "action=award.import&awardname=The%20Pack%20Design&awardtype=Design%20Trophy&classname=ThePack" | check_success
curl_post action.php "action=award.import&awardname=Bad%20Subgroup%20Name&awardtype=Design%20Trophy&subgroup=Typo" | check_failure

curl_post action.php "action=award.winner&awardid=1&racerid=2" | check_failure
curl_post action.php "action=award.winner&awardid=1&racerid=1" | check_success

curl_post action.php "action=settings.write&n-rank-trophies=3" | check_success

curl_post action.php "action=award.present&key=speed-3-1-2" | check_success
curl_get "action.php?query=award.current" | expect_one '3rd Fastest in Tigers'
curl_get "action.php?query=award.current" | expect_one Thanh

curl_post action.php "action=award.present&key=speed-3-1" | check_success
curl_get "action.php?query=award.current" | expect_one '3rd Fastest in ThePack'
curl_get "action.php?query=award.current" | expect_one Jefferys

curl_post action.php "action=award.present&key=speed-3" | check_success
curl_get "action.php?query=award.current" | expect_one '3rd Fastest in Pack'
curl_get "action.php?query=award.current" | expect_one Jefferys
