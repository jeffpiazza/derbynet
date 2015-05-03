#! /bin/bash

# Simulate the Replay application with nc, so we can observe (and
# participate in) the traffic between the server and the Replay app.

BASE_URL=$1
set -e -E -o pipefail
source `dirname $0`/common.sh

# Send a register-replay action for port 50000 to the server
user_login RaceCoordinator doyourbest
sleep 1 && ( curl_post action.php "action=register-replay&port=50000" ) &

echo Respond to replay messages with an \"OK\"
nc -c -k -l 50000
