#! /bin/bash

BASE_URL=$1
set -e -E -o pipefail
source `dirname $0`/common.sh

RESET_SOURCE=messaging `dirname $0`/reset-database.sh "$BASE_URL"
curl_postj action.php "action=database.execute&script=drop-message-queue-table" | check_jsuccess

# This is the weird application/x-www-form-urlencoded encoding for:
# message={"recipient": "AA", "key": "first-value"}
curl_postj action.php "action=message.send&message%5Brecipient%5D=AA&message%5Bkey%5D=first-value" | check_jsuccess
curl_postj action.php "action=message.retrieve&recipient=BB" | \
    jq '.messages | length' | expect_eq 0
curl_postj action.php "action=message.retrieve&recipient=AA" | \
    jq '.messages | length == 1 and .[0].key == "first-value"' | expect_eq true
curl_postj action.php "action=message.retrieve&recipient=AA" | \
    jq '.messages | length' | expect_eq 0
