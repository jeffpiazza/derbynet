#! /bin/sh

BASE_URL=$1
set -e -E -o pipefail
source `dirname $0`/common.sh

if [ "$2" = "" ]; then
    echo "Usage: <URL root> <photo path>"
    exit
fi

# curl_post action.php
curl --location -s -b $COOKIES_CURL -c $COOKIES_CURL $BASE_URL/action.php \
    -X POST -F photo="@$2" -F action=upload-photo

