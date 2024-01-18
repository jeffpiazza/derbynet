#! /bin/bash

BASE_URL=$1
set -e -E -o pipefail
source `dirname $0`/common.sh

touch /tmp/cleanup
rm -rf /tmp/headshots* /tmp/carphotos* /tmp/cleanup

function upload_headshot() {
    curl --location -s -b $COOKIES_CURL -c $COOKIES_CURL $BASE_URL/action.php \
         -X POST -F MAX_FILE_SIZE=2000000 -F photo="@$1" -F action=photo.upload -F repo=head \
    | tee $DEBUG_CURL | check_jsuccess
}

function upload_car_photo() {
    curl --location -s -b $COOKIES_CURL -c $COOKIES_CURL $BASE_URL/action.php \
         -X POST -F MAX_FILE_SIZE=2000000 -F photo="@$1" -F action=photo.upload -F repo=car \
    | tee $DEBUG_CURL | check_jsuccess
}


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

user_login_coordinator

# Delete a photo, then upload it again
curl_photo_any head/file/original/Cub-3126.jpg/xyz
curl_postj action.php "action=photo.delete&repo=head&photo=Cub-3126.jpg" | check_jsuccess

COUNT=`curl --location -s -b $COOKIES_CURL -c $COOKIES_CURL $BASE_URL/photo.php/head/file/original/Cub-3126.jpg/xyz | wc -c`
if [ $COUNT -gt 1000 ]; then
    test_fails Photo not deleted
fi
upload_headshot `dirname $0`/data/headshots/Cub-3126.jpg
