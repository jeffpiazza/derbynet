#! /bin/bash

BASE_URL=$1
set -e -E -o pipefail
source `dirname $0`/common.sh

`dirname $0`/reset-database.sh "$BASE_URL"
curl_postj action.php "action=database.execute&script=drop-message-queue-table" | check_jsuccess

curl_postj action.php "action=message.send&recipient=AA&message=%7B%22key%22%3A%20%22first-value%22%7D" | check_jsuccess
curl_postj action.php "action=message.retrieve&recipient=BB" | \
    jq '.messages | length' | expect_eq 0
curl_postj action.php "action=message.retrieve&recipient=AA" | \
    jq '.messages | length == 1 and .[0].key == "first-value"' | expect_eq true
curl_postj action.php "action=message.retrieve&recipient=AA" | \
    jq '.messages | length' | expect_eq 0
