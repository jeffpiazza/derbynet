#! /bin/sh

BASE_URL=$1
set -e -E -o pipefail
source `dirname $0`/common.sh

if [ "$2" = "" ]; then
    echo "Usage: <URL root> \"query=xyz&param1=...\""
    exit
fi

curl_get "action.php?$2"
