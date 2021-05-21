#! /bin/bash

if [ "$2" = "" -o "$4" != "" ]; then
    echo "Usage: <URL root> <photo path> [<racerid>]"
    exit
fi

BASE_URL=$1
set -e -E -o pipefail
source `dirname $0`/common.sh

PHOTO=$2
RACERID=$3

user_login_photo

curl --location -s -b $COOKIES_CURL -c $COOKIES_CURL $BASE_URL/action.php \
    -X POST -F photo="@$PHOTO" -F action=json.photo.upload -F racerid="$RACERID" -F autocrop=1 # | check_jsuccess

