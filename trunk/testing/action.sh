#! /bin/sh

BASE_URL=$1

if [ "$BASE_URL" = "" ]; then
	echo Base URL required!
	exit
fi

if [ "$2" = "" ]; then
    echo "Usage: <URL root> \"action=xyz&param1=...\""
    exit
fi

source common.sh

curl_post action.php "$2"
