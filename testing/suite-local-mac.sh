#! /bin/bash

BASE_URL=$1
set -e -E -o pipefail
source `dirname $0`/common.sh

`dirname $0`/test-ab-initio-setup.sh "$BASE_URL"
`dirname $0`/exercise-one-database.sh "$BASE_URL" sqlite
