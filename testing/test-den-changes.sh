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

CLASSID=$(grep Bears $CLASS_LIST | grep '<class' | sed -e 's/^.* classid="\([^"]*\)".*$/\1/')

RANKID=$(sed -n -e  "/<class.*classid=.$CLASSID. /,/<\/class>/ s/^.* rankid=.\\([0-9]*\\).*$/\\1/p" $CLASS_LIST)

# Can't delete the only rank in a class
curl_post action.php "action=rank.delete&rankid=$RANKID" | check_failure

curl_post action.php "action=rank.add&classid=$CLASSID&name=SecondRank" | check_success
RANKID2=$(curl_get "action.php?query=class.list" | grep SecondRank | sed "s/^.* rankid=.\\([0-9]\\).*$/\\1/")

curl_post action.php "action=rank.order&rankid_1=$RANKID2&rankid_2=$RANKID" | check_success
curl_post action.php "action=rank.edit&rankid=$RANKID2&name=New%20Rank%20Name" | check_success

GOLDEN=$(mktemp /tmp/derby-golden-class.list.XXXXX)
cat >$GOLDEN <<EOF
<?xml version="1.0" encoding="UTF-8"?>
<classes_and_ranks>
  <class classid="1" count="17" nrounds="0" name="Lions &amp; Tigers">
    <rank rankid="1" count="17" name="Lions &amp; Tigers"/>
  </class>
  <class classid="2" count="17" nrounds="0" name="White's Wolves">
    <rank rankid="2" count="17" name="White's Wolves"/>
  </class>
  <class classid="3" count="16" nrounds="0" name="Bears and Frèr">
    <rank rankid="8" count="0" name="New Rank Name"/>
    <rank rankid="3" count="16" name="Bears and Frèr"/>
  </class>
  <class classid="4" count="16" nrounds="0" name="Webelos (&quot;Webes">
    <rank rankid="4" count="16" name="Webelos (&quot;Webes"/>
  </class>
  <class classid="5" count="16" nrounds="0" name="Arrows &lt;&lt;--&lt;&lt;">
    <rank rankid="5" count="16" name="Arrows &lt;&lt;--&lt;&lt;"/>
  </class>
  <class classid="7" count="0" nrounds="0" name="TheLastClass">
    <rank rankid="7" count="0" name="TheLastClass"/>
  </class>
</classes_and_ranks>
EOF

curl_get "action.php?query=class.list" > $CLASS_LIST
diff $GOLDEN $CLASS_LIST || test_fails "class.list fails"

curl_post action.php "action=rank.delete&rankid=$RANKID2" | check_success

