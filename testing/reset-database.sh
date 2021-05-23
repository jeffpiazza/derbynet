#! /bin/bash

BASE_URL=$1
set -e -E -o pipefail
source `dirname $0`/common.sh

user_login_coordinator

# Capture settings for photo directories and restore them in the new database


PHOTO_DIR=$(curl_getj "action.php?query=settings.list&key=photo-directory" | \
                jq -r 'if .settings | length > 0 then .settings[0].value else "" end')

CAR_PHOTO_DIR=$(curl_getj "action.php?query=settings.list&key=car-photo-directory" | \
                    jq -r 'if .settings | length > 0 then .settings[0].value else "" end')

VIDEO_DIR=$(curl_getj "action.php?query=settings.list&key=video-directory" | \
                jq -r 'if .settings | length > 0 then .settings[0].value else "" end')

curl_postj action.php "action=database.execute&script=schema" | check_jsuccess
curl_postj action.php "action=database.execute&script=update-schema" | check_jsuccess

if [ "$PHOTO_DIR" ] ; then
    curl_postj action.php "action=settings.write&photo-dir=$PHOTO_DIR" | check_jsuccess
fi
if [ "$CAR_PHOTO_DIR" ] ; then
    curl_postj action.php "action=settings.write&car-photo-dir=$CAR_PHOTO_DIR" | check_jsuccess
fi
if [ "$VIDEO_DIR" ] ; then
    curl_postj action.php "action=settings.write&video-dir=$VIDEO_DIR" | check_jsuccess
fi

