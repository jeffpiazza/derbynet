#! /bin/bash

BASE_URL=$1
shift
DBTYPE=$1
shift

set -e -E -o pipefail
source `dirname $0`/common.sh

# Force to a test sqlite database -- don't accidentally clobber the current database!
DBPATH=${1:-/Library/WebServer/Documents/xsite/local/trial.sqlite}

user_login_coordinator

curl_post action.php \
        "action=setup.nodata&connection_string=sqlite:$DBPATH&dbuser=&dbpass=" \
        | check_success


`dirname $0`/reset-database.sh "$BASE_URL"
`dirname $0`/import-roster.sh "$BASE_URL"
`dirname $0`/test-den-changes.sh "$BASE_URL"

`dirname $0`/photo-setup.sh "$BASE_URL"
`dirname $0`/test-photo-upload.sh "$BASE_URL"

# `dirname $0`/test-awards.sh "$BASE_URL" basic
`dirname $0`/test-photo-assignments.sh "$BASE_URL"

curl_post action.php "action=settings.write&n-lanes=4" | check_success
