#! /bin/bash

BASE_URL=$1
set -e -E -o pipefail
source `dirname $0`/common.sh

user_login_coordinator

user_login_crew

curl --location -s -b $COOKIES_CURL -c $COOKIES_CURL $BASE_URL/action.php \
    -X POST -F action=json.photo.upload -F photo="@`dirname $0`/data/ISO_12233-reschart.jpg" | tee $DEBUG_CURL | check_jsuccess

curl_photo_any head/file/original/ISO_12233-reschart.jpg/$RANDOM

curl_postj action.php "action=json.photo.assign&repo=head&photo=ISO_12233-reschart.jpg&racer=30" | check_jsuccess

# As of PHP 5.5, this gives a different signature
# curl_photo head/file/100x100/ISO_12233-reschart.jpg/$RANDOM 16329a387284cfbb7ece2f2a6a173040
# curl_photo head/file/100x100/ISO_12233-reschart.jpg/$RANDOM 4e61175e45a4beb93aeb20973ebdc04d
# curl_photo head/file/100x100/ISO_12233-reschart.jpg/$RANDOM 0c6589fafa70aa6f44dfb054753ce347
# PHP 5.5.30:
curl_photo_any head/file/100x100/ISO_12233-reschart.jpg/$RANDOM

curl_postj action.php "action=json.photo.crop&repo=head&image_name=ISO_12233-reschart.jpg&left=100&top=150&right=190&bottom=270&original_height=480&original_width=900" | check_jsuccess
# curl_photo head/file/original/ISO_12233-reschart.jpg/$RANDOM b539242a66779a3721d7eb0fd6d2a9c9
# curl_photo head/file/cropped/ISO_12233-reschart.jpg/$RANDOM cd9d674652844e7f7cdd083358a4def4
# curl_photo head/file/100x100/ISO_12233-reschart.jpg/$RANDOM a6f23a5f3ba4b90b157c75cc9532bd61
# curl_photo head/file/100x100/ISO_12233-reschart.jpg/$RANDOM 5f7ff77c76740dc5ce88a100b550121e
# curl_photo head/file/100x100/ISO_12233-reschart.jpg/$RANDOM f7392996d5d93207b571a2768eea9e2e
# PHP 5.5.30:
curl_photo_any head/file/100x100/ISO_12233-reschart.jpg/$RANDOM

curl_postj action.php "action=json.photo.rotate&repo=head&image_name=ISO_12233-reschart.jpg&rotation=90" | check_jsuccess


curl_photo_any head/file/original/ISO_12233-reschart.jpg/$RANDOM

curl_postj action.php "action=json.photo.rotate&repo=head&image_name=ISO_12233-reschart.jpg&rotation=-90" | check_jsuccess
# Rotating left and then right doesn't give you back the exact same image, due to compression losses in jpg

curl_photo_any head/file/original/ISO_12233-reschart.jpg/$RANDOM
curl_photo_any head/file/cropped/ISO_12233-reschart.jpg/$RANDOM

curl_photo_any head/racer/30/$RANDOM
