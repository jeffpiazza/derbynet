#! /bin/sh

BASE_URL=$1

if [ "$BASE_URL" = "" ]; then
	echo Base URL required!
	exit
fi

if [ "$2" = "" ]; then
    echo "Usage: <URL root> <photo path>"
    exit
fi

source `dirname $0`/common.sh

# curl_post action.php
curl --location -s -b $COOKIES_CURL -c $COOKIES_CURL $BASE_URL/action.php \
    -X POST -F photo="@$2" -F action=upload-photo

