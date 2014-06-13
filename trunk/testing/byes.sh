#! /bin/sh

BASE_URL=$1

if [ "$BASE_URL" = "" ]; then
	echo Base URL required!
	exit
fi

source common.sh

# TODO: Relies on cookies.curl

curl_post action.php "action=pass&racer=33" | check_success
curl_post action.php "action=pass&racer=34" | check_success

curl_post action.php "action=schedule&roundid=5" | check_success
curl_post action.php "action=advance-heat&roundid=5&heat=1" | check_success
