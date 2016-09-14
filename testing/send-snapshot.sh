#! /bin/bash

BASE_URL=$1
set -e -E -o pipefail
source `dirname $0`/common.sh

if [ "$2" = "" -o "$3" != "" ]; then
    echo "Usage: <URL root> <snapshot path>"
    exit
fi

curl --location -s -b $COOKIES_CURL -c $COOKIES_CURL $BASE_URL/action.php \
    -X POST -F snapshot="@$2" -F action=snapshot.put

