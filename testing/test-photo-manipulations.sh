#! /bin/bash

BASE_URL=$1
set -e -E -o pipefail
source `dirname $0`/common.sh

if [ ! `echo "$BASE_URL" | grep -i localhost` ]; then
    tput setaf 2  # green text
    echo Skipping photo manipulation tests
    tput setaf 0  # black text
    exit 0
fi

tmpdir=`mktemp -d 2>/dev/null || mktemp -d /tmp/photo_uploads.XXXXXXXX`
chmod 777 $tmpdir

`dirname $0`/login-coordinator.sh "$BASE_URL"

curl_post action.php "action=write-settings&photo-dir=$tmpdir" | check_success
curl_post action.php "action=write-settings&photo-width=188&photo-height=250" | check_success

user_login RaceCrew murphy

curl --location -s -b $COOKIES_CURL -c $COOKIES_CURL $BASE_URL/action.php \
    -X POST -F action=photo.upload -F photo="@`dirname $0`/data/ISO_12233-reschart.jpg" | tee $DEBUG_CURL | check_success

curl_photo head/file/original/ISO_12233-reschart.jpg/$RANDOM bdbcb19aa69c2e1d4db8bc05d803bb3c

curl_post action.php "action=photo.rotate&repo=head&image_name=ISO_12233-reschart.jpg&rotation=90" | check_success
curl_photo head/file/original/ISO_12233-reschart.jpg/$RANDOM 20f65ded3929019ac30a57cfb86473c0
curl_post action.php "action=photo.rotate&repo=head&image_name=ISO_12233-reschart.jpg&rotation=-90" | check_success
# Rotating left and then right doesn't give you back the exact same image, due to compression losses in jpg
curl_photo head/file/original/ISO_12233-reschart.jpg/$RANDOM b539242a66779a3721d7eb0fd6d2a9c9
curl_photo head/file/cropped/ISO_12233-reschart.jpg/$RANDOM b539242a66779a3721d7eb0fd6d2a9c9
curl_photo head/file/display/ISO_12233-reschart.jpg/$RANDOM 16329a387284cfbb7ece2f2a6a173040

curl_post action.php "action=photo.crop&repo=head&image_name=ISO_12233-reschart.jpg&left=100&top=150&right=190&bottom=270&original_height=480&original_width=900" | check_success
curl_photo head/file/original/ISO_12233-reschart.jpg/$RANDOM b539242a66779a3721d7eb0fd6d2a9c9
curl_photo head/file/cropped/ISO_12233-reschart.jpg/$RANDOM cd9d674652844e7f7cdd083358a4def4
curl_photo head/file/display/ISO_12233-reschart.jpg/$RANDOM a6f23a5f3ba4b90b157c75cc9532bd61

# Clean up temporary directory
rm -rf $tmpdir
