#! /bin/bash

BASE_URL="$1"

set -e -E -o pipefail

# Puppeteer tests
# https://github.com/GoogleChrome/puppeteer

echo " " " " " " all-pages-test.js
node "`dirname $0`/puppeteer/all-pages-test.js" "$BASE_URL"
echo " " " " " " coordinator-test.js
node "`dirname $0`/puppeteer/coordinator-test.js" "$BASE_URL"
echo " " " " " " ondeck-columns-test.js
node "`dirname $0`/puppeteer/ondeck-columns-test.js" "$BASE_URL"


