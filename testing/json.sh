#! /bin/bash

BASE_URL=$1
set -e -E -o pipefail
source `dirname $0`/common.sh

if [ "$2" = "" ]; then
    echo "Usage: <URL root> \"query=xyz&param1=...\""
    exit
fi

curl_json "action.php?$2"
