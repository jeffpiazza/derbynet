#! /bin/sh

mkdir -m 777 /Library/WebServer/Documents/derbynet/local

mkdir -m 775 "$HOME/Public/DerbyNet"
chown $USER:_www "$HOME/Public/DerbyNet"

# V2.0 didn't include JUDGING_PERMISSION for race crew in the default
# config-roles file.  Delete and recreate the config-roles file.
test -e /Library/WebServer/Documents/derbynet/local/config-roles.inc && \
grep -q -v JUDGING_PERMISSION /Library/WebServer/Documents/derbynet/local/config-roles.inc && \
    sed -i.bak -e 's/ASSIGN_RACER_IMAGE_PERMISSION/ASSIGN_RACER_IMAGE_PERMISSION | JUDGING_PERMISSION/' \
        /Library/WebServer/Documents/derbynet/local/config-roles.inc
