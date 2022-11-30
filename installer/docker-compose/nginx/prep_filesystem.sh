#!/bin/bash
WWW_ROOT=/usr/share/nginx/html
if [ -d "$WWW_ROOT" ]
then
    if [ "$(ls -A $WWW_ROOT)" ]; then
        echo "$WWW_ROOT is not Empty, using existing structure."
    else
        echo "$WWW_ROOT is Empty, creating DerbyNet Website filesystem."
        cd /
        echo "Cloning Jeff's repo."
        git clone https://github.com/jeffpiazza/derbynet.git
        echo "Moving the website files into $WWW_ROOT."
        mv /derbynet/website/* $WWW_ROOT/.
        echo "Moving the missing banner.inc file into place."
        mv /derbynet/templates/banner.inc $WWW_ROOT/inc/banner.inc
        echo "Getting rid of the Derbynet repo to conserve space."
        rm -rf /derbynet
        echo "Fixing permissions on $WWW_ROOT/local"
        test -d $WWW_ROOT/local || mkdir -m 777 $WWW_ROOT/local
    fi
else
	echo "Directory $DIR not found."
fi