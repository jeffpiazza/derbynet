#! /bin/bash

BASE_URL=$1
set -e -E -o pipefail
source `dirname $0`/common.sh

if [ "$2" = "" -o "$4" != "" ]; then
    echo "Usage: <URL root> <photo path> [<racerid>]"
    exit
fi

curl --location -s -b $COOKIES_CURL -c $COOKIES_CURL $BASE_URL/action.php \
    -X POST -F photo="@$2" -F action=photo.upload -F racerid="$3" | check_success

