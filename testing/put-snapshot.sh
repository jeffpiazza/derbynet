#! /bin/bash

BASE_URL="$1"
SNAPSHOT="$2"

set -e -E -o pipefail
source `dirname $0`/common.sh

user_login_coordinator
curl_put_snapshot "$SNAPSHOT" | check_success

