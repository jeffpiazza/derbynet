#! /bin/sh

BASE_URL=$1

if [ "$BASE_URL" = "" ]; then
	echo Base URL required!
	exit
fi

if [ "$2" = "" ]; then
    echo "Usage: <URL root> \"query=xyz&param1=...\""
    exit
fi

source `dirname $0`/common.sh

curl_get "action.php?$2"
