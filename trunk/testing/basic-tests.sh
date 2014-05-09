#! /bin/sh

BASE_URL=$1

# TODO: Exercise update.php
# TODO: Exercise checkin-action.php
# TODO: Verify sort options for checkin.php

# Search each page for all its links?
# <form method="link" action="...">
# <a href="...">

cat >anonymous.index.tmp <<EOF
        <form method="link" action="ondeck.php">
        <form method="link" action="racer-results.php">
        <form method="link" action="login.php">
EOF

cat >coordinator.index.tmp <<EOF
        <form method="link" action="checkin.php">
        <form method="link" action="photo-thumbs.php">
        <form method="link" action="ondeck.php">
        <form method="link" action="racer-results.php">
        <form method="link" action="awards.php">
        <form method="link" action="settings.php">
        <form method="link" action="utilities.php">
        <form method="link" action="login.php">
EOF

cat >racecrew.index.tmp <<EOF
        <form method="link" action="checkin.php">
        <form method="link" action="ondeck.php">
        <form method="link" action="racer-results.php">
        <form method="link" action="awards.php">
        <form method="link" action="login.php">
EOF

rm cookies.curl
rm output.curl
CURL="curl -L -s -b cookies.curl -c cookies.curl"

# TODO: Consider --trace or --trace-ascii

function curl_get() {
	echo '     ' $1 >&2
	echo    >> output.curl
	echo $1 >> output.curl
	echo    >> output.curl
	curl --location -s -b cookies.curl -c cookies.curl $BASE_URL/$1 | tee debug.curl \
		| xmllint --format - | tee -a output.curl
}

function curl_get_amper() {
	echo '     ' $1 >&2
	echo    >> output.curl
	echo $1 >> output.curl
	echo    >> output.curl
	curl --location -s -b cookies.curl -c cookies.curl $BASE_URL/$1 | tee debug.curl \
		| grep -v '&' | xmllint --format - | tee -a output.curl
}

function curl_post() {
	echo '     post ' $1 $2 >&2
	echo    >> output.curl
	echo post $1 $2 >> output.curl
	echo    >> output.curl
	curl --location -d $2 -s -b cookies.curl -c cookies.curl $BASE_URL/$1 | tee debug.curl \
		| xmllint --format - | tee -a output.curl
}

function user_login() {
	# $1 = user name
	# $2 = password
	[ "`curl_post login-action.php "name=$1&password=$2" | sed -ne 's!.*<success>\(.*\)</success>!\1!p'`" = "$1" ] \
		|| login_as $1 failed!
}

function user_logout() {
	[ "`curl_get login-action.php | sed -ne 's!.*<success>\(.*\)</success>!\1!p'`" = "" ] \
        || user_logout failed
}

curl_get index.php | grep '<form' | diff - anonymous.index.tmp
[ "`curl_get test/create-database.php | grep 'Database script completed!'`" ] || echo Database creation failed

user_login RaceCoordinator doyourbest

echo '     Populate database'
./populate-database.sh $BASE_URL
[ `curl_get checkin.php | grep -c '<td>Owen</td>'` -eq 1 ] || echo Owen O\'Connor!

user_logout


curl_get ondeck.php | \
	sed -ne 's/.*href="\([^"]*\)".*/\1/p' | grep -v racer-results.php | grep -v ondeck.css | grep -v '#group_' | \
	diff - /dev/null

[ `curl_get "current.php?since=2013-11-14%2014:57:14" | grep -c 'completed="2013-11-14 14:57:14"'` -eq 1 ] || echo current.php fails!


curl_get racer-results.php | \
	sed -ne 's/.*href="\([^"]*\)".*/\1/p' | grep -v 'ondeck.php#heat' | grep -v ondeck.css | grep -v '#group_' | \
	diff - /dev/null

curl_get "racer-results.php?racerid=35" | \
	sed -ne 's/.*href="\([^"]*\)".*/\1/p' | grep -v 'ondeck.php#heat' | grep -v ondeck.css | grep -v '#group_' | \
	diff - /dev/null

curl_get_amper login.php | sed -ne 's/.*href="\([^"]*\)".*/\1/p' | grep -v ondeck.css | diff - /dev/null

user_login RaceCoordinator doyourbest
curl_get index.php | grep '<form' | diff - coordinator.index.tmp

[ `curl_get checkin.php | grep -c '<tr '` -eq 50 ] || echo Checkin!
# POST                      : action=xbs&racer=<racerid>&value=<1/0>
# POST action=renumber&racer=<racerid>&value=<carnumber>
# POST action=classchange&racer=<racerid>&value=<rankid>
# POST action=photo&racer=<racerid>&photo=<filepath> and maybe &previous=<old-racerid>
# POST action=initaudit
# POST action=initnumbers

[ `curl_get settings.php | grep -c '<input'` -eq 15 ] || echo Settings!
# TODO: Verify settings behavior

[ `curl_get utilities.php | grep -c '<input'` -eq 3 ] || echo Utilities!
# TODO: Verify utilities behavior

# Because there have been no races, there are no entries on the awards page...
[ `curl_get awards.php | grep -c '<tr'` -eq 26 ] || echo Awards! `curl_get awards.php | grep -c '<tr'`

#        <form method="link" action="photo-thumbs.php">

user_logout
curl_get index.php | grep '<form' | diff - anonymous.index.tmp

user_login RaceCrew murphy
curl_get index.php | grep '<form' | diff - racecrew.index.tmp
user_logout

curl_get index.php | grep '<form' | diff - anonymous.index.tmp

# Catch any "undefined" messages
grep -i Undefined output.curl
