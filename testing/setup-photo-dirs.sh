#! /bin/bash

BASE_URL=$1
set -e -E -o pipefail
source `dirname $0`/common.sh

touch /tmp/cleanup
rm -rf /tmp/headshots* /tmp/carphotos* /tmp/cleanup



if [ `echo "$BASE_URL" | grep -i localhost` ]; then
    PHOTO_DIR=`mktemp -d /tmp/headshots.XXXXXXXX`
    # Need world write access to allow web host to create subfolders
    chmod 777 "$PHOTO_DIR"

    # CAR_PHOTO_DIR=`mktemp -d 2>/dev/null || mktemp -d /tmp/carphotos.XXXXXXXX`
    CAR_PHOTO_DIR=`mktemp -d /tmp/carphotos.XXXXXXXX`
    chmod 777 "$CAR_PHOTO_DIR"

    VIDEO_DIR=`mktemp -d /tmp/videos.XXXXXXXX`
    chmod 777 "$VIDEO_DIR"

    user_login_coordinator

    curl_postj action.php "action=settings.write&photo-dir=$PHOTO_DIR" | check_jsuccess
    curl_postj action.php "action=settings.write&car-photo-dir=$CAR_PHOTO_DIR" | check_jsuccess
    curl_postj action.php "action=settings.write&video-dir=$VIDEO_DIR" | check_jsuccess
    curl_postj action.php "action=settings.write&photos-on-now-racing=head" | check_jsuccess
fi
