#! /bin/bash

BASE_URL=$1
set -e -E -o pipefail
source `dirname $0`/common.sh

`dirname $0`/reset-database.sh "$BASE_URL"
curl_post action.php "action=database.execute&script=drop-message-queue-table" | check_success

curl_post action.php "action=message.send&recipient=AA&message=first-message" | check_success
curl_post action.php "action=message.retrieve&recipient=BB" | expect_count "<message" 0
curl_post action.php "action=message.retrieve&recipient=AA" | grep "<message" | expect_count ">first-message</message>" 1
curl_post action.php "action=message.retrieve&recipient=AA" | expect_count "<message" 0
