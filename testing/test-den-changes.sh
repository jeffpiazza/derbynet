#! /bin/bash

BASE_URL=$1
set -e -E -o pipefail
source `dirname $0`/common.sh

curl_post action.php "action=class.add&name=TheNotLastClass" | check_success

# Regression test: class.add left the sortorder field unpopulated
curl_post action.php "action=class.add&name=TheLastClass" | check_success

CLASS_LIST=$(mktemp /tmp/derby-class.list.XXXXX)
curl_get "action.php?query=class.list" > $CLASS_LIST
if [ "`tail -3 $CLASS_LIST | head -1 | grep -c TheLastClass`" -ne 1 ]; then
    test_fails New class should be sorted last
fi

CLASSID=$(grep TheNotLastClass $CLASS_LIST | grep '<class' | sed -e 's/^.* classid="\([^"]*\)".*$/\1/')
curl_post action.php "action=class.delete&classid=$CLASSID" | check_success

