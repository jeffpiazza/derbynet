#! /bin/bash

BASE_URL=$1
set -e -E -o pipefail
source `dirname $0`/common.sh

curl_getj "action.php?query=racer.list" | \
    jq '.racers[0].lastname == "Akiyama"' | expect_eq true

curl_getj "action.php?query=racer.list&order=name" | \
    jq '.racers[0].lastname == "Akiyama"' | expect_eq true
curl_getj "action.php?query=racer.list&order=class" | \
    jq '.racers[0].lastname == "Asher"' | expect_eq true
curl_getj "action.php?query=racer.list&order=rank" | \
    jq '.racers[0].lastname == "Bittinger"' | expect_eq true
curl_getj "action.php?query=racer.list&order=car" | \
    jq '.racers[0].lastname == "Asher"' | expect_eq true
curl_getj "action.php?query=racer.list&order=checkin" | \
    jq '.racers[0].lastname == "Fizer"' | expect_eq true
