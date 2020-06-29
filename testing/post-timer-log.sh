#! /bin/sh
# Reads data from stdin

BASE_URL=$1
set -e -E -o pipefail
source `dirname $0`/common.sh

curl -X POST -H 'Content-Type: text/plain' --data-binary @- \
     $BASE_URL/timer-log.php
