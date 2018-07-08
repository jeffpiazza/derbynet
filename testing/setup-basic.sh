#! /bin/bash

BASE_URL=$1
shift
DBTYPE=$1
shift

set -e -E -o pipefail
source `dirname $0`/common.sh

`dirname $0`/reset-database.sh "$BASE_URL"
`dirname $0`/import-roster.sh "$BASE_URL"
`dirname $0`/test-den-changes.sh "$BASE_URL"

`dirname $0`/photo-setup.sh "$BASE_URL"
`dirname $0`/test-photo-upload.sh "$BASE_URL"

`dirname $0`/test-basic-racing.sh "$BASE_URL"

`dirname $0`/test-awards.sh "$BASE_URL" basic
`dirname $0`/test-photo-assignments.sh "$BASE_URL"
