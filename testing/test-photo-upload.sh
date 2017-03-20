#! /bin/bash

BASE_URL=$1
set -e -E -o pipefail
source `dirname $0`/common.sh

# Test uploading photos with assignment to racer

# $1: path to photo
# $2: param=value for selecting racer
function upload_carphoto_to_racer() {
    echo ' ' ' ' ' photo-upload' $1 with $2 >&2
    echo                         >> $OUTPUT_CURL
    echo photo-upload $1 with $2 >> $OUTPUT_CURL
    echo                         >> $OUTPUT_CURL
    curl --location -s -b $COOKIES_CURL -c $COOKIES_CURL $BASE_URL/action.php \
         -X POST -F MAX_FILE_SIZE=2000000 -F photo="@$1" \
         -F "$2" -F action=photo.upload -F repo=car \
        | tee $DEBUG_CURL | check_success
}

# Racerid 65 Porter Papke, Car 465
upload_carphoto_to_racer "`dirname $0`/data/carphotos/Text-Car-465.jpg" barcode=PWDid65
# Racerid 66 Raymon Ruffner, Car 166
upload_carphoto_to_racer "`dirname $0`/data/carphotos/Text-Car-166.jpg" racerid=66
# Racerid 67 Renaldo Raposo, Car 267
upload_carphoto_to_racer "`dirname $0`/data/carphotos/Text-Car-267.jpg" carnumber=267

exit 0
