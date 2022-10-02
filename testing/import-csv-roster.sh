#! /bin/bash

BASE_URL=$1
CSV_FILE=$2

# Firstname, lastname, car number, partition

set -e -E -o pipefail
source `dirname $0`/common.sh

grep -v '^#' "$CSV_FILE" | grep -v "^$" | \
    while IFS="," read -r first last carno part ; do
        # grp=${grp//&/%26}

        echo ${first//\"/} ${last//\"/}, ${carno//\"/}, ${part//\"/}

	    curl --location \
             -d         "action=racer.import" \
             -d         "firstname=${first//\"/}" \
             -d         "lastname=${last//\"/}" \
             -d         "carnumber=${carno//\"/}" \
             --data-raw "partition=${part//\"/}" \
             -s -b $COOKIES_CURL -c $COOKIES_CURL $BASE_URL/action.php \
            | tee $DEBUG_CURL \
		    | tee -a $OUTPUT_CURL \
            | check_jsuccess
    done
