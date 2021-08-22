#! /bin/bash

BASE_URL=$1
set -e -E -o pipefail
source `dirname $0`/common.sh

user_login_coordinator

`dirname $0`/reset-database.sh "$BASE_URL"

curl_postj action.php "action=settings.write&do-use-subgroups=1&do-use-subgroups-checkbox=1" | check_jsuccess

curl_postj action.php "action=division.apply-rule&rule=one-group" | check_jsuccess

# Single group, ThePack
# Subgroup Lions = 1
# Subgroup Tigers = 2
# Subgroup Bears = 3
# Subgroup Wolves = 4

curl_postj action.php "action=racer.import&firstname=Jewell&lastname=Jeansonne&carnumber=101&division=Lions&exclude=" | check_jsuccess
curl_postj action.php "action=racer.import&firstname=Cedrick&lastname=Charley&carnumber=202&division=Tigers" | check_jsuccess
curl_postj action.php "action=racer.import&firstname=Grant&lastname=Gribble&carnumber=303&division=Bears" | check_jsuccess
curl_postj action.php "action=racer.import&firstname=Harrison&lastname=Hanks&carnumber=404&division=Wolves" | check_jsuccess
curl_postj action.php "action=racer.import&firstname=Jerald&lastname=Jerry&carnumber=105&division=Lions&exclude=X" | check_jsuccess
curl_postj action.php "action=racer.import&firstname=Ernest&lastname=Edelman&carnumber=206&division=Tigers" | check_jsuccess
curl_postj action.php "action=racer.import&firstname=Lloyd&lastname=Lightsey&carnumber=307&division=Bears" | check_jsuccess
curl_postj action.php "action=racer.import&firstname=Gary&lastname=Grissom&carnumber=408&division=Wolves" | check_jsuccess
curl_postj action.php "action=racer.import&firstname=Von&lastname=Vassar&carnumber=109&division=Lions&exclude=NO" | check_jsuccess
curl_postj action.php "action=racer.import&firstname=Harold&lastname=Hayek&carnumber=210&division=Tigers&exclude=yes" | check_jsuccess
curl_postj action.php "action=racer.import&firstname=Pasquale&lastname=Procopio&carnumber=311&division=Bears" | check_jsuccess
curl_postj action.php "action=racer.import&firstname=Winford&lastname=Weld&carnumber=412&division=Wolves" | check_jsuccess
curl_postj action.php "action=racer.import&firstname=John&lastname=Jefferys&carnumber=113&division=Lions&exclude=0" | check_jsuccess
curl_postj action.php "action=racer.import&firstname=Thanh&lastname=Turner&carnumber=214&division=Tigers" | check_jsuccess
curl_postj action.php "action=racer.import&firstname=Demetrius&lastname=Demming&carnumber=315&division=Bears" | check_jsuccess
curl_postj action.php "action=racer.import&firstname=Everette&lastname=Esses&carnumber=416&division=Wolves" | check_jsuccess
curl_postj action.php "action=racer.import&firstname=Forrest&lastname=Figgins&carnumber=117&division=Lions" | check_jsuccess
curl_postj action.php "action=racer.import&firstname=Sterling&lastname=Spalla&carnumber=218&division=Tigers" | check_jsuccess
curl_postj action.php "action=racer.import&firstname=Delmar&lastname=Donnelly&carnumber=319&division=Bears" | check_jsuccess
curl_postj action.php "action=racer.import&firstname=Stanton&lastname=Salmon&carnumber=420&division=Wolves" | check_jsuccess

curl_postj action.php "action=racer.pass&value=1&racer=1" | check_jsuccess
curl_postj action.php "action=racer.pass&value=1&racer=2" | check_jsuccess
curl_postj action.php "action=racer.pass&value=1&racer=3" | check_jsuccess
curl_postj action.php "action=racer.pass&value=1&racer=4" | check_jsuccess
curl_postj action.php "action=racer.pass&value=1&racer=5" | check_jsuccess
curl_postj action.php "action=racer.pass&value=1&racer=6" | check_jsuccess
curl_postj action.php "action=racer.pass&value=1&racer=7" | check_jsuccess
curl_postj action.php "action=racer.pass&value=1&racer=8" | check_jsuccess
curl_postj action.php "action=racer.pass&value=1&racer=9" | check_jsuccess
curl_postj action.php "action=racer.pass&value=1&racer=10" | check_jsuccess
curl_postj action.php "action=racer.pass&value=1&racer=11" | check_jsuccess
curl_postj action.php "action=racer.pass&value=1&racer=12" | check_jsuccess
curl_postj action.php "action=racer.pass&value=1&racer=13" | check_jsuccess
curl_postj action.php "action=racer.pass&value=1&racer=14" | check_jsuccess
curl_postj action.php "action=racer.pass&value=1&racer=15" | check_jsuccess
curl_postj action.php "action=racer.pass&value=1&racer=16" | check_jsuccess
curl_postj action.php "action=racer.pass&value=1&racer=17" | check_jsuccess
curl_postj action.php "action=racer.pass&value=1&racer=18" | check_jsuccess
curl_postj action.php "action=racer.pass&value=1&racer=19" | check_jsuccess
curl_postj action.php "action=racer.pass&value=1&racer=20" | check_jsuccess

curl_postj action.php "action=settings.write&n-lanes=4" | check_jsuccess
curl_postj action.php "action=schedule.generate&roundid=1" | check_jsuccess

curl_postj action.php "action=heat.select&roundid=1&now_racing=1" | check_jsuccess

# Fast Lions    1.100
# Slow Tigers   5.500
# Mixed Bears, Wolves 2.00, 2.50, 3.00

# Racing chart (1=Lions, etc.):

user_login_timer
curl_post action.php "action=timer-message&message=HELLO" | check_success
curl_post action.php "action=timer-message&message=IDENTIFIED&nlanes=4" | check_success
cat $DEBUG_CURL | expect_one "<heat-ready[ />]"

run_heat 1  1  101:1.010 303:2.503 117:1.170 408:2.508
run_heat 1  2  202:5.502 404:2.504 218:5.518 109:1.09
run_heat 1  3  303:2.503 105:1.050 319:2.519 210:5.510
run_heat 1  4  404:2.504 206:5.506 420:3.020 311:3.011
run_heat 1  5  105:1.050 307:2.507 101:1.010 412:3.012
run_heat 1  6  113:1.130 315:3.015 109:1.090 420:3.020
run_heat 1  7  412:3.012 214:5.514 408:2.508 319:2.519
run_heat 1  8  311:3.011 113:1.130 307:2.507 218:5.518
run_heat 1  9  214:5.514 416:3.016 210:5.510 101:1.01
run_heat 1 10  315:3.015 117:1.170 311:3.011 202:5.502
run_heat 1 11  416:3.016 218:5.518 412:3.012 303:2.503
run_heat 1 12  206:5.506 408:2.508 202:5.502 113:1.13
run_heat 1 13  307:2.507 109:1.090 303:2.503 214:5.514
run_heat 1 14  408:2.508 210:5.510 404:2.504 315:3.015
run_heat 1 15  109:1.090 311:3.011 105:1.050 416:3.016
run_heat 1 16  210:5.510 412:3.012 206:5.506 117:1.17
run_heat 1 17  218:5.518 420:3.020 214:5.514 105:1.05
run_heat 1 18  117:1.170 319:2.519 113:1.130 404:2.504
run_heat 1 19  420:3.020 202:5.502 416:3.016 307:2.507
run_heat 1 20  319:2.519 101:1.010 315:3.015 206:5.506 x


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
curl_text "standings.php" | grep '<tr' | grep "data-rankid=.2." | grep "col-insubgroup.>4<" | expect_one "Sterling Spalla"

curl_getj "action.php?query=standings.reveal" | expect_count catalog-entry 0
# javascript: encodeURIComponent(JSON.stringify({kind: 'supergroup', key: 'supergroup', name: 'Pack'}))
curl_postj action.php "action=standings.reveal&catalog-entry=%7B%22kind%22%3A%22supergroup%22%2C%22key%22%3A%22supergroup%22%2C%22name%22%3A%22Pack%22%7D" | \
    jq -r 'if .outcome.summary == "success" then .["catalog-entry"] else . end' | \
    jq '.kind == "supergroup" and .key == "supergroup" and .name == "Pack"' | \
    expect_eq true

curl_postj action.php "action=standings.reveal&catalog-entry=%7B%22kind%22%3A%22class%22%2C%22key%22%3A%22c1%22%2C%22name%22%3A%22One Group%22%7D" | \
    jq -r 'if .outcome.summary == "success" then .["catalog-entry"] else . end' | \
    jq '.kind == "class" and .key == "c1" and .name == "One Group"' | \
    expect_eq true
curl_postj action.php "action=standings.reveal&catalog-entry=%7B%22kind%22%3A%22rank%22%2C%22key%22%3A%22r2%22%2C%22name%22%3A%22Tigers%22%7D" | \
    jq -r 'if .outcome.summary == "success" then .["catalog-entry"] else . end' | \
    jq '.kind == "rank" and .key == "r2" and .name == "Tigers"' | \
    expect_eq true

# Rank-specific awards:

curl_postj action.php "action=award.import&awardname=Most%20Lionic&awardtype=Design%20General&subgroup=Lions" | check_jsuccess
curl_postj action.php "action=award.import&awardname=Most%20Tigerific&awardtype=Other&subgroup=Tigers" | check_jsuccess
curl_postj action.php "action=award.import&awardname=The%20Pack%20Design&awardtype=Design%20Trophy" | check_jsuccess
curl_postj action.php "action=award.import&awardname=Bad%20Subgroup%20Name&awardtype=Design%20Trophy&subgroup=Typo" | check_jfailure

curl_postj action.php "action=award.winner&awardid=1&racerid=2" | check_jfailure
curl_postj action.php "action=award.winner&awardid=1&racerid=1" | check_jsuccess

curl_postj action.php "action=settings.write&n-rank-trophies=3" | check_jsuccess

curl_postj action.php "action=award.present&key=speed-3-1-2" | check_jsuccess
curl_getj "action.php?query=award.current" | expect_one '3rd Fastest in Tigers'
curl_getj "action.php?query=award.current" | expect_one Thanh

curl_postj action.php "action=award.present&key=speed-3-1" | check_jsuccess
curl_getj "action.php?query=award.current" | expect_one '3rd Fastest in One Group'
curl_getj "action.php?query=award.current" | expect_one Jefferys

curl_postj action.php "action=award.present&key=speed-3" | check_jsuccess
curl_getj "action.php?query=award.current" | expect_one '3rd Fastest in Pack'
curl_getj "action.php?query=award.current" | expect_one Jefferys
