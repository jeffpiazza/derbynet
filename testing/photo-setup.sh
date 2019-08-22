#! /bin/bash

BASE_URL=$1
set -e -E -o pipefail
source `dirname $0`/common.sh

touch /tmp/cleanup
rm -rf /tmp/headshots* /tmp/carphotos* /tmp/cleanup

function upload_headshot() {
    curl --location -s -b $COOKIES_CURL -c $COOKIES_CURL $BASE_URL/action.php \
         -X POST -F MAX_FILE_SIZE=2000000 -F photo="@$1" -F action=photo.upload -F repo=head \
    | tee $DEBUG_CURL | check_success
}

function upload_car_photo() {
    curl --location -s -b $COOKIES_CURL -c $COOKIES_CURL $BASE_URL/action.php \
         -X POST -F MAX_FILE_SIZE=2000000 -F photo="@$1" -F action=photo.upload -F repo=car \
    | tee $DEBUG_CURL | check_success
}

if [ `echo "$BASE_URL" | grep -i localhost` ]; then
    # For localhost, just manufacture a tmp directory and copy files locally
    tput setaf 2  # green text
    echo "    " Copying photo files instead of uploading "(localhost)"
    tput setaf 0  # black text

    PHOTO_DIR=`mktemp -d /tmp/headshots.XXXXXXXX`
    # Need world write access to allow web host to create subfolders
    chmod 777 "$PHOTO_DIR"
    cp `dirname $0`/data/headshots/Cub* "$PHOTO_DIR"
    cp `dirname $0`/data/headshots/head* "$PHOTO_DIR"

    # CAR_PHOTO_DIR=`mktemp -d 2>/dev/null || mktemp -d /tmp/carphotos.XXXXXXXX`
    CAR_PHOTO_DIR=`mktemp -d /tmp/carphotos.XXXXXXXX`
    chmod 777 "$CAR_PHOTO_DIR"
    cp `dirname $0`/data/carphotos/Car* "$CAR_PHOTO_DIR"

    VIDEO_DIR=`mktemp -d /tmp/videosXXXXXXXX`
    chmod 777 "$VIDEO_DIR"

    user_login_coordinator

    curl_post action.php "action=settings.write&photo-dir=$PHOTO_DIR" | check_success
    curl_post action.php "action=settings.write&car-photo-dir=$CAR_PHOTO_DIR" | check_success
    curl_post action.php "action=settings.write&photos-on-now-racing=head" | check_success
else
    # For the remote case, assume that directories have been set up, and upload each photo
    echo Headshot uploads begin

    user_login_photo

    for f in `dirname $0`/data/headshots/head-*.jpg `dirname $0`/data/headshots/Cub-*.jpg
    do
        echo $f
        upload_headshot $f
    done

    echo Car photo uploads begin
    for f in `dirname $0`/data/carphotos/Car-*.jpg
    do
        echo $f
        upload_car_photo $f
    done

    echo Done
fi
