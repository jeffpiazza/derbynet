#! /bin/bash

BASE_URL=$1
set -e -E -o pipefail
source `dirname $0`/common.sh

user_login_coordinator

# Capture settings for photo directories and restore them in the new database
PHOTO_DIR=$(curl_get "action.php?query=settings" | xmllint --format - | sed -ne '/key="photo-directory"/ s#[^>]*>\([^<]*\)</setting>#\1#p')

CAR_PHOTO_DIR=$(curl_get "action.php?query=settings" | xmllint --format - | sed -ne '/key="car-photo-directory"/ s#[^>]*>\([^<]*\)</setting>#\1#p')

VIDEO_DIR=$(curl_get "action.php?query=settings" | xmllint --format - | sed -ne '/key="video-directory"/ s#[^>]*>\([^<]*\)</setting>#\1#p')

curl_post action.php "action=database.execute&script=schema" | check_success
curl_post action.php "action=database.execute&script=update-schema" | check_success

if [ "$PHOTO_DIR" ] ; then
    curl_post action.php "action=settings.write&photo-dir=$PHOTO_DIR" | check_success
fi
if [ "$CAR_PHOTO_DIR" ] ; then
    curl_post action.php "action=settings.write&car-photo-dir=$CAR_PHOTO_DIR" | check_success
fi
if [ "$VIDEO_DIR" ] ; then
    curl_post action.php "action=settings.write&video-dir=$VIDEO_DIR" | check_success
fi

