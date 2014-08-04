#! /bin/sh

BASE_URL=$1

if [ "$BASE_URL" = "" ]; then
	echo Base URL required!
	exit
fi

# TODO: Exercise update.js
# TODO: Exercise checkin-action.php
# TODO: Verify sort options for checkin.php

# Search each page for all its links, to make sure they all work?
# <a href="...">
# <form method="link" action="...">

source common.sh

[ -e cookies.curl ] && rm cookies.curl
[ -e output.curl ] && rm output.curl
# debug.curl gets overwritten

curl_get index.php | grep '<form' | diff - anonymous.index.tmp

[ "`curl_get create-database.php | grep 'Database script completed!'`" ] || echo Database creation failed

user_login RaceCoordinator doyourbest

echo ' ' ' ' ' ' 'Populate database'
./populate-database.sh $BASE_URL
[ `curl_get checkin.php | grep -c '>Owen</td>'` -eq 1 ] || echo Owen O\'Connor!
# TODO: There's a 15-character limit on class names and rank names.
curl_post action.php "action=import&lastname=Flintstone&firstname=Fred&classname=6-Cartoons&carnumber=425" | check_success

user_logout


curl_get ondeck.php | \
	sed -ne 's/.*href="\([^"]*\)".*/\1/p' | grep -v racer-results.php | grep -v ondeck.css | grep -v '#group_' | \
	diff - /dev/null

[ `curl_get "action.php?query=update-summary&since=2013-11-14%2014:57:14" | grep -c 'completed="2013-11-14 14:57:14"'` -eq 1 ] || echo update_summary fails!


curl_get racer-results.php | \
	sed -ne 's/.*href="\([^"]*\)".*/\1/p' | grep -v 'ondeck.php#heat' | grep -v ondeck.css | grep -v '#group_' | \
	diff - /dev/null

curl_get "racer-results.php?racerid=35" | \
	sed -ne 's/.*href="\([^"]*\)".*/\1/p' | grep -v 'ondeck.php#heat' | grep -v ondeck.css | grep -v '#group_' | \
	diff - /dev/null

curl_get "kiosk.php?page=kiosks/welcome.kiosk" > /dev/null
curl_get "kiosk.php?page=kiosks/identify.kiosk" > /dev/null
curl_get "kiosk.php?page=kiosks/please_check_in.kiosk" > /dev/null
curl_get "kiosk.php?page=kiosks/now-racing.kiosk" > /dev/null

curl_get "action.php?query=classes" > /dev/null

curl_get_amper login.php | sed -ne 's/.*href="\([^"]*\)".*/\1/p' | grep -v ondeck.css | diff - /dev/null

user_login RaceCoordinator doyourbest
curl_get index.php | grep '<form' | diff - coordinator.index.tmp

[ `curl_get checkin.php | grep -c '<tr '` -eq 51 ] || echo Checkin!

curl_post action.php "action=schedule&roundid=1" | check_success
curl_post action.php "action=schedule&roundid=2" | check_success
curl_post action.php "action=schedule&roundid=3" | check_success

curl_post action.php "action=pass&racer=8&value=1" | check_success
curl_post action.php "action=schedule&roundid=1" | check_success
curl_post action.php "action=pass&racer=8&value=0" | check_success

# TODO: delete-results only wipes out the times, not the fact that a schedule exists.
#curl_post action.php "action=schedule&roundid=1" | check_success

curl_post action.php "action=edit-racer&racer=2&firstname=Tom&lastname=Adrogue&carno=5011&rankid=3" | check_success
[ `curl_get checkin.php | grep firstname-2 | grep -c '>Tom</td>'` -eq 1 ] || echo Firstname change
[ `curl_get checkin.php | grep '"class-2"' | grep -c '>3 - Bear</td>'` -eq 1 ] || echo Car rank change
[ `curl_get checkin.php | grep -c '>5011</td>'` -eq 1 ] || echo Car number change

# POST action=photo&racer=<racerid>&photo=<filepath> and maybe &previous=<old-racerid>
# POST action=initaudit
# POST action=initnumbers

[ `curl_get settings.php | grep -c '<input'` -eq 15 ] || echo Settings!
# TODO: Verify settings behavior

# [ `curl_get utilities.php | grep -c '<input'` -eq 3 ] || echo Utilities!
# TODO: Verify utilities behavior

# Because there have been no races, there are no entries on the awards page...
[ `curl_get awards.php | grep -c '<tr'` -eq 26 ] || echo Awards! `curl_get awards.php | grep -c '<tr'`

#        <form method="link" action="photo-thumbs.php">

curl_post action.php "action=schedule&roundid=2" | check_success "scheduling round 2"
curl_post action.php "action=schedule&roundid=3" | check_success "scheduling round 3"
curl_post action.php "action=schedule&roundid=4" | check_success "scheduling round 4"
# The 5th round won't schedule, because there's only one racer.
curl_post action.php "action=schedule&roundid=5" | check_failure "scheduling round 5"

sleep 2

curl_post action.php "action=advance-heat&roundid=1&heat=1" | check_success "setting initial heat"

curl_post action.php "action=heat-results&lane1=3.000&lane2=3.001&lane3=3.010&lane4=3.100" | check_success heat
curl_post action.php "action=heat-results&lane1=3.000&lane2=3.001&lane3=3.010&lane4=3.100" | check_success heat
curl_post action.php "action=heat-results&lane1=3.000&lane2=3.001&lane3=3.010&lane4=3.100" | check_success heat
curl_post action.php "action=heat-results&lane1=3.100&lane2=3.001&lane3=3.110&lane4=3.200" | check_success heat
curl_post action.php "action=heat-results&lane1=3.000&lane2=3.001&lane3=3.010&lane4=3.100" | check_success heat
sleep 1
curl_post action.php "action=heat-results&lane1=3.000&lane2=3.001&lane3=3.010&lane4=3.100" | check_success heat
curl_post action.php "action=heat-results&lane4=2.999&lane2=3.001&lane3=3.010&lane1=9.999" | check_success heat
sleep 1
curl_post action.php "action=heat-results&lane4=2.999&lane2=3.001&lane3=3.010&lane1=9.999" | check_success heat

curl_post action.php "action=delete-results&roundid=2" | check_success

# Un-check-in a bunch of Wolves, to leave BYEs in the schedule
curl_post action.php "action=pass&racer=5&value=0" | check_success
curl_post action.php "action=pass&racer=11&value=0" | check_success
curl_post action.php "action=pass&racer=17&value=0" | check_success
curl_post action.php "action=pass&racer=23&value=0" | check_success
curl_post action.php "action=pass&racer=31&value=0" | check_success

# Re-generate the schedule.
curl_post action.php "action=schedule&roundid=2" | check_success

# TODO There seems to be a timing hazard if the first heat results come too quickly
sleep 2
# Testing heat-results with BYEs
curl_post action.php "action=heat-results&lane4=3.123&lane1=3.210" | check_success
curl_post action.php "action=heat-results&lane1=3.123&lane2=3.210" | check_success
curl_post action.php "action=heat-results&lane2=3.123&lane3=3.210" | check_success
curl_post action.php "action=heat-results&lane3=3.123&lane4=3.210" | check_success

# TODO: Testing some rescheduling
curl_post action.php "action=pass&racer=4&value=1" | check_success check-in
curl_post action.php "action=schedule&roundid=3" | check_success schedule
curl_post action.php 'action=advance-heat&roundid=3' | check_success select_round
curl_post action.php "action=heat-results&lane1=3.100&lane2=3.001&lane3=3.110&lane4=3.200" | check_success heat
sleep 1
curl_post action.php "action=pass&racer=40&value=1" | check_success check-in
sleep 1
curl_post action.php "action=reschedule&roundid=3" | check_success reschedule

user_logout
curl_get index.php | grep '<form' | diff - anonymous.index.tmp

user_login RaceCrew murphy
curl_get index.php | grep '<form' | diff - racecrew.index.tmp
user_logout

curl_get index.php | grep '<form' | diff - anonymous.index.tmp

rm *.index.tmp

tput setaf 2
echo Tests complete

tput setaf 1
# Catch any "undefined" messages
grep -i Undefined output.curl
tput setaf 0

