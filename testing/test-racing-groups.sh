#! /bin/bash

BASE_URL=$1
set -e -E -o pipefail
source `dirname $0`/common.sh

RESET_SOURCE=racing-groups `dirname $0`/reset-database.sh "$BASE_URL"

# Empty database behavior
curl_getj "action.php?query=poll&values=race-structure" | \
    jq -e '.["group-formation-rule"] == "by-partition"' > /dev/null || test_fails

curl_getj "action.php?query=poll&values=classes" | \
    jq -e '.classes | length == 0' > /dev/null || test_fails
curl_getj "action.php?query=poll&values=partitions" | \
    jq -e '.partitions | length == 0' > /dev/null || test_fails

curl_postj action.php "action=partition.apply-rule&rule=one-group" | check_jsuccess

curl_getj "action.php?query=poll&values=classes" | \
    jq -e '.classes | length == 1' > /dev/null || test_fails
curl_getj "action.php?query=poll&values=partitions" | \
    jq -e '.partitions | length == 0' > /dev/null || test_fails

curl_postj action.php "action=partition.apply-rule&rule=custom" | check_jsuccess

curl_getj "action.php?query=poll&values=classes" | \
    jq -e '.classes | length == 0' > /dev/null || test_fails
curl_getj "action.php?query=poll&values=partitions" | \
    jq -e '.partitions | length == 0' > /dev/null || test_fails


`dirname $0`/import-roster.sh "$BASE_URL"

# Should already be by-partition, but for consistency:
curl_postj action.php "action=partition.apply-rule&rule=by-partition" | check_jsuccess


# BY_, ONE_, CUS_ prefix
# PART, RANK, CLASS
# _X
# _RANK (the rankid not directly created)

# 


# Add, rename, delete partition
curl_postj action.php "action=partition.add&name=By-Part-X" | check_jsuccess
BY_PART_X=$(partitionid_of "By-Part-X")

BY_PART_X_RANK=$(rankid_of "By-Part-X")
[[ "$BY_PART_X_RANK" = null ]] && test_fails No rankid for new partition
BY_PART_X_CLASS=$(classid_of "By-Part-X")
[[ "$BY_PART_X_CLASS" = null ]] && test_fails No classid for new partition

curl_postj action.php "action=partition.edit&partitionid=$BY_PART_X&name=By-Part-ModX" | \
    check_jsuccess
[[ "$BY_PART_X" = $(partitionid_of "By-Part-ModX") ]] || test_fails
[[ "$BY_PART_X_RANK" = $(rankid_of "By-Part-ModX") ]] || test_fails
[[ "$BY_PART_X_CLASS" = $(classid_of "By-Part-ModX") ]] || test_fails

curl_postj action.php "action=partition.delete&partitionid=$BY_PART_X" | check_jsuccess
[[ $(partitionid_of "By-Part-ModX") = null ]] || test_fails
[[ $(rankid_of "By-Part-ModX") = null ]] || test_fails
[[ $(classid_of "By-Part-ModX") = null ]] || test_fails

# Add, rename, delete rank
## We don't provide a mechanism for creating, renaming, or deleting a rank while in by-partition

# Add, rename, delete class
## We don't provide a way to create a group in by-partition, but rename or delete are possible.
curl_postj action.php "action=partition.add&name=By-Part-Y" | check_jsuccess
BY_PART_Y=$(partitionid_of "By-Part-Y")
BY_PART_Y_RANK=$(rankid_of "By-Part-Y")
[[ "$BY_PART_Y_RANK" = null ]] && test_fails Now rankid for new class
BY_PART_Y_CLASS=$(classid_of "By-Part-Y")
[[ "$BY_PART_Y_CLASS" = null ]] && test_fails Now classid for new class

curl_postj action.php "action=class.edit&classid=$BY_PART_Y_CLASS&name=By-Part-ModY" | check_jsuccess
[[ "$BY_PART_Y" = $(partitionid_of "By-Part-ModY") ]] || test_fails
[[ "$BY_PART_Y_RANK" = $(rankid_of "By-Part-ModY") ]] || test_fails
[[ "$BY_PART_Y_CLASS" = $(classid_of "By-Part-ModY") ]] || test_fails

curl_postj action.php "action=class.delete&classid=$BY_PART_Y_CLASS" | check_jsuccess
[[ $(partitionid_of "By-Part-ModY") = null ]] || test_fails
[[ $(rankid_of "By-Part-ModY") = null ]] || test_fails
[[ $(classid_of "By-Part-ModY") = null ]] || test_fails

#######################################################################################
curl_postj action.php "action=partition.apply-rule&rule=one-group" | check_jsuccess
ONEGROUPID=$(classid_of "All Racers")

# Add, rename, delete partition
curl_postj action.php "action=partition.add&name=One-Part-X" | check_jsuccess
ONE_PART_X=$(partitionid_of "One-Part-X")
ONE_PART_X_RANK=$(rankid_of "One-Part-X")
[[ "$ONE_PART_X_RANK" = null ]] && test_fails No rankid for new partition
# Shouldn't get a new class for a new partition under one-group
[[ $(classid_of "One-Part-X") = null ]] || test_fails

curl_postj action.php "action=partition.edit&partitionid=$ONE_PART_X&name=One-Part-ModX" | \
    check_jsuccess
[[ "$ONE_PART_X" = $(partitionid_of "One-Part-ModX") ]] || test_fails
[[ "$ONE_PART_X_RANK" = $(rankid_of "One-Part-ModX") ]] || test_fails
[[ $(classid_of "One-Part-ModX") = null ]] || test_fails

curl_postj action.php "action=partition.delete&partitionid=$ONE_PART_X" | check_jsuccess
[[ $(partitionid_of "One-Part-ModX") = null ]] || test_fails
[[ $(rankid_of "One-Part-ModX") = null ]] || test_fails

# Add, rename, delete rank
## We don't provide a mechanism for creating a rank directly in one-group
ONE_PART_LION=$(partitionid_of "Lion")
ONE_RANK_LION=$(rankid_of "Lion")
curl_postj action.php "action=rank.edit&rankid=$ONE_RANK_LION&name=One-Rank-ModLion" | check_jsuccess
[[ "$ONE_PART_LION" = $(partitionid_of "One-Rank-ModLion") ]] || test_fails
[[ "$ONE_RANK_LION" = $(rankid_of "One-Rank-ModLion") ]] || test_fails
[[ $(classid_of "One-Rank-ModLion") = null ]] || test_fails

# Need an empty partition and rank to test deleting a rank
curl_postj action.php "action=partition.add&name=One-Part-Y" | check_jsuccess
ONE_PART_Y=$(partitionid_of "One-Part-Y")
ONE_PART_Y_RANK=$(rankid_of "One-Part-Y")
[[ "$ONE_PART_Y_RANK" = null ]] && test_fails No rankid for new partition
# Shouldn't get a new class for a new partition under one-group
[[ $(classid_of "One-Part-Y") = null ]] || test_fails

curl_postj action.php "action=rank.delete&rankid=$ONE_PART_Y_RANK" | check_jsuccess
[[ $(partitionid_of "One-Part-Y") = null ]] || test_fails
[[ $(rankid_of "One-Part-Y") = null ]] || test_fails
[[ $(classid_of "One-Part-Y") = null ]] || test_fails

# Add, rename, delete class
## We don't provide a way to create or delete a group in one-class, but rename is certainly possible
curl_postj action.php "action=class.edit&classid=$ONEGROUPID&name=Pack1234" | check_jsuccess
[[ "$ONEGROUPID" = $(classid_of "Pack1234") ]] || test_fails
[[ "$ONE_PART_LION" = $(partitionid_of "One-Rank-ModLion") ]] || test_fails
[[ "$ONE_RANK_LION" = $(rankid_of "One-Rank-ModLion") ]] || test_fails


#######################################################################################
curl_postj action.php "action=partition.apply-rule&rule=by-partition" | check_jsuccess
curl_postj action.php "action=partition.apply-rule&rule=custom" | check_jsuccess

# Add, rename, delete partition
curl_postj action.php "action=partition.add&name=Cus-Part-X" | check_jsuccess
CUS_PART_X=$(partitionid_of "Cus-Part-X")
CUS_PART_X_RANK=$(rankid_of "Cus-Part-X")
[[ "$CUS_PART_X_RANK" = null ]] && test_fails No rankid for new partition
CUS_PART_X_CLASS=$(classid_of "Cus-Part-X")
[[ "$CUS_PART_X_CLASS" = null ]] && test_fails No classid for new partition

curl_postj action.php "action=partition.edit&partitionid=$CUS_PART_X&name=Cus-Part-ModX" | \
    check_jsuccess
[[ "$CUS_PART_X" = $(partitionid_of "Cus-Part-ModX") ]] || test_fails
[[ "$CUS_PART_X_RANK" = $(rankid_of "Cus-Part-ModX") ]] || test_fails
[[ $(classid_of "Cus-Part-ModX") = null ]] || test_fails
[[ $(rankid_of "Cus-Part-X") = null ]] || test_fails
[[ "$CUS_PART_X_CLASS" = $(classid_of "Cus-Part-X") ]] || test_fails

curl_postj action.php "action=partition.delete&partitionid=$CUS_PART_X" | check_jsuccess
[[ $(partitionid_of "Cus-Part-ModX") = null ]] || test_fails
[[ $(rankid_of "Cus-Part-ModX") = null ]] || test_fails
[[ $(classid_of "Cus-Part-ModX") = null ]] || test_fails
[[ $(rankid_of "Cus-Part-X") = null ]] || test_fails
[[ "$CUS_PART_X_CLASS" = $(classid_of "Cus-Part-X") ]] || test_fails

# Add, rename, delete rank
CUS_CLASS_BEAR=$(classid_of "Bear")
curl_postj action.php "action=rank.add&classid=$CUS_CLASS_BEAR&name=Cus-Rank-Y" | check_jsuccess
[[ $(partitionid_of "Cus-Rank-Y") = null ]] || test_fails
CUS_RANK_Y=$(rankid_of "Cus-Rank-Y")
[[ "$CUS_RANK_Y" = null ]] && test_fails
CUS_RANK_Y_CLASS=$(classid_of "Cus-Rank-Y")
[[ "$CUS_RANK_Y_CLASS" = null ]] || test_fails

curl_postj action.php "action=rank.edit&rankid=$CUS_RANK_Y&name=Cus-Rank-ModY" | check_jsuccess
[[ $(partitionid_of "Cus-Rank-ModY") = null ]] || test_fails
[[ "$CUS_RANK_Y" = $(rankid_of "Cus-Rank-ModY") ]] || test_fails
[[ "$CUS_RANK_Y_CLASS" = $(classid_of "Cus-Rank-Y") ]] || test_fails

curl_postj action.php "action=rank.delete&rankid=$CUS_RANK_Y" | check_jsuccess
[[ $(partitionid_of "Cus-Rank-ModY") = null ]] || test_fails
[[ $(rankid_of "Cus-Rank-Y") = null ]] || test_fails
[[ $(rankid_of "Cus-Rank-ModY") = null ]] || test_fails
[[ $(classid_of "Cus-Rank-Y") = null ]] || test_fails
[[ $(classid_of "Cus-Rank-ModY") = null ]] || test_fails

# Add, rename, delete class
curl_postj action.php "action=class.add&name=Cus-Class-Z" | check_jsuccess
CUS_RANK_Z=$(rankid_of "Cus-Class-Z")
[[ "$CUS_RANK_Z" = null ]] && test_fails
CUS_CLASS_Z=$(classid_of "Cus-Class-Z")
[[ "$CUS_CLASS_Z" = null ]] && test_fails

curl_postj action.php "action=class.edit&classid=$CUS_CLASS_Z&name=Cus-Class-ModZ" | check_jsuccess
[[ $(partitionid_of "Cus-Class-ModZ") = null ]] || test_fails
[[ "$CUS_RANK_Z" = $(rankid_of "Cus-Class-Z") ]] || test_fails rank $CUS_RANK_Z
[[ "$CUS_CLASS_Z" = $(classid_of "Cus-Class-ModZ") ]] || test_fails

curl_postj action.php "action=class.delete&classid=$CUS_CLASS_Z" | check_jsuccess
[[ $(classid_of "Cus-Class-Z") = null ]] || test_fails
[[ $(classid_of "Cus-Class-ModZ") = null ]] || test_fails


