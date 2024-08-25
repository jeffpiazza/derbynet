#! /bin/bash

BASE_URL="$1"
if [ -z "$BASE_URL" ] ; then
  BASE_URL=localhost
fi

set -e -E -o pipefail
source `dirname $0`/common.sh

`dirname $0`/js-syntax-check.sh
`dirname $0`/test-ab-initio-setup.sh "$BASE_URL"
`dirname $0`/exercise-one-database.sh "$BASE_URL" ez

# There's no action for deleting a database and/or data directory, so we do it
# by hand for the local instance.
CONFIGS_DIR=~/Public/DerbyNet
rm -rf $CONFIGS_DIR/$(date +%Y)/testdb
# If there's no config, it's not possible to log in, so we leave the database configured
#mv $CONFIGS_DIR/config-database.inc $CONFIGS_DIR/x-config-database.inc
rm $CONFIGS_DIR/config-database.inc
#mv $CONFIGS_DIR/config-roles.inc $CONFIGS_DIR/x-config-roles.inc
rm $CONFIGS_DIR/config-roles.inc
