#! /bin/bash

BASE_URL=$1
shift
if [ -z "$BASE_URL" ] ; then
  BASE_URL=localhost
fi


set -e -E -o pipefail
source `dirname $0`/common.sh

user_login_coordinator

curl_postj action.php "action=setup.nodata&ez-new=testdb" > /dev/null

`dirname $0`/reset-database.sh "$BASE_URL"
`dirname $0`/import-roster.sh "$BASE_URL"
`dirname $0`/test-den-changes.sh "$BASE_URL"

curl_postj action.php "action=settings.write&n-lanes=4" | check_jsuccess
