#! /bin/bash

BASE_URL=$1
set -e -E -o pipefail
source `dirname $0`/common.sh

curl_postj action.php "action=racer.pass&racer=1&value=1" | check_jsuccess
curl_postj action.php "action=racer.pass&racer=3&value=1" | check_jsuccess
curl_postj action.php "action=racer.pass&racer=5&value=1" | check_jsuccess
curl_postj action.php "action=racer.pass&racer=7&value=1" | check_jsuccess
curl_postj action.php "action=racer.pass&racer=9&value=1" | check_jsuccess
curl_postj action.php "action=racer.pass&racer=11&value=1" | check_jsuccess
curl_postj action.php "action=racer.pass&racer=13&value=1" | check_jsuccess
curl_postj action.php "action=racer.pass&racer=15&value=1" | check_jsuccess
curl_postj action.php "action=racer.pass&racer=17&value=1" | check_jsuccess
#curl_postj action.php "action=racer.pass&racer=19&value=1" | check_jsuccess
curl_postj action.php "action=racer.pass&racer=21&value=1" | check_jsuccess
curl_postj action.php "action=racer.pass&racer=23&value=1" | check_jsuccess
curl_postj action.php "action=racer.pass&racer=25&value=1" | check_jsuccess
curl_postj action.php "action=racer.pass&racer=27&value=1" | check_jsuccess
#curl_postj action.php "action=racer.pass&racer=29&value=1" | check_jsuccess
curl_postj action.php "action=racer.pass&racer=31&value=1" | check_jsuccess
curl_postj action.php "action=racer.pass&racer=33&value=1" | check_jsuccess
curl_postj action.php "action=racer.pass&racer=35&value=1" | check_jsuccess
##curl_postj action.php "action=racer.pass&racer=37&value=1" | check_jsuccess
curl_postj action.php "action=racer.pass&barcode=PWD237&value=1" | check_jsuccess
##curl_postj action.php "action=racer.pass&racer=39&value=1" | check_jsuccess
curl_postj action.php "action=racer.pass&barcode=PWD508&value=1" | check_jsuccess
curl_postj action.php "action=racer.pass&racer=41&value=1" | check_jsuccess
curl_postj action.php "action=racer.pass&racer=43&value=1" | check_jsuccess
curl_postj action.php "action=racer.pass&racer=45&value=1" | check_jsuccess
curl_postj action.php "action=racer.pass&racer=47&value=1" | check_jsuccess
curl_postj action.php "action=racer.pass&racer=49&value=1" | check_jsuccess

# Exclude car 111
curl_postj action.php "action=racer.edit&racer=11&firstname=Carroll&lastname=Cybulski&carno=111&carname=Vroom&rankid=1&exclude=1" | check_jsuccess

# Change a racer's den
curl_postj action.php "action=racer.edit&racer=39&firstname=Jeffress&lastname=Jamison&carno=139&carname=&rankid=3" | check_jsuccess

# Reject partitionid -1
curl_postj action.php "action=racer.edit&racer=45&firstname=Savvy&lastname=Savant&carno=445&carname=&note_from=&partitionid=-1&exclude=0" | check_jfailure

# TODO: There was a bug that the new-row returned from this was basically empty; this doesn't test that
#
# After changing a den, adding a new racer used to be a bug (extra Roster row).
# The effect was observable extra entries for heats that included the
# den-changed racer.
curl_postj action.php "action=racer.add&firstname=Fred&lastname=Flintstone&carno=789&carname=&partitionid=2&note_from=Bedrock" | check_jsuccess
curl_getj "action.php?query=racer.list" | \
    jq -e '.racers | map(select(.carnumber == 789))[0].note_from == "Bedrock"' > /dev/null || test_fails
