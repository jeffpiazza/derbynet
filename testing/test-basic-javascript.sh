#! /bin/bash

BASE_URL="$1"

set -e -E -o pipefail

PHANTOMJS="$(which phantomjs)"
if [ ! -x "$PHANTOMJS" ] ; then
    echo Skipping phantomjs tests
    exit
fi

echo " " " " " " test-basic-javascript

echo "[]" > "`dirname $0`/cookies.phantomjs"
phantomjs "`dirname $0`/phantomjs/test-all-pages.js" "$BASE_URL"
phantomjs "`dirname $0`/phantomjs/login.js" "$BASE_URL"
phantomjs "`dirname $0`/phantomjs/test-all-pages.js" "$BASE_URL"

# Puppeteer tests
# https://github.com/GoogleChrome/puppeteer
echo " " " " " " coordinator-test.js
node "`dirname $0`/puppeteer/coordinator-test.js" "$BASE_URL"
echo " " " " " " ondeck-columns-test.js
node "`dirname $0`/puppeteer/ondeck-columns-test.js" "$BASE_URL"

echo " " " " " " Done with test-basic-javascript
