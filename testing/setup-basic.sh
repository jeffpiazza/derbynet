#! /bin/bash

BASE_URL=$1
shift
DBTYPE=$1
shift

set -e -E -o pipefail
source `dirname $0`/common.sh

# Force to a test sqlite database -- don't accidentally clobber the current database!
DBPATH=${1:-/Library/WebServer/Documents/xsite/local/trial.sqlite}

`dirname $0`/setup-basic-no-photos.sh "$BASE_URL"

`dirname $0`/photo-setup.sh "$BASE_URL"
`dirname $0`/test-photo-upload.sh "$BASE_URL"

`dirname $0`/test-photo-assignments.sh "$BASE_URL"

curl_postj action.php "action=settings.write&n-lanes=4" | check_jsuccess
