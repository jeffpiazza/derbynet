#! /bin/bash

BASE_URL="$1"

set -e -E -o pipefail

PHANTOMJS="$(which phantomjs)"
if [ ! -x "$PHANTOMJS" ] ; then
    echo Skipping phantomjs tests
    exit
fi

echo "[]" > "`dirname $0`/cookies.phantomjs"
phantomjs "`dirname $0`/phantomjs/test-all-pages.js" "$BASE_URL"
phantomjs "`dirname $0`/phantomjs/login.js" "$BASE_URL"
phantomjs "`dirname $0`/phantomjs/test-all-pages.js" "$BASE_URL"
phantomjs "`dirname $0`/phantomjs/test-ondeck-columns.js" "$BASE_URL"
