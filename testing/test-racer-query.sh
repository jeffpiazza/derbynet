#! /bin/bash

BASE_URL=$1
set -e -E -o pipefail
source `dirname $0`/common.sh

curl_get "action.php?query=racer.list" | grep -e '<racer ' | head -n 1 | expect_one Akiyama || true

curl_get "action.php?query=racer.list&order=name" | grep '<racer ' | head -n 1 | expect_one Akiyama || true
curl_get "action.php?query=racer.list&order=class" | grep '<racer ' | head -n 1 | expect_one Asher || true
curl_get "action.php?query=racer.list&order=rank" | grep '<racer ' | head -n 1 | expect_one Bittinger || true
curl_get "action.php?query=racer.list&order=car" | grep '<racer ' | head -n 1 | expect_one Asher || true
curl_get "action.php?query=racer.list&order=checkin" | grep '<racer ' | head -n 1 | expect_one Fizer || true
