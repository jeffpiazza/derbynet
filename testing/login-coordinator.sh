#! /bin/sh

BASE_URL=$1

if [ "$BASE_URL" = "" ]; then
	echo Base URL required!
	exit
fi

source `dirname $0`/common.sh

user_login RaceCoordinator doyourbest
