#!/bin/bash
WWW_ROOT=/usr/share/nginx/html
if [ -d "$WWW_ROOT" ]
then
    if [ "$(ls -A $WWW_ROOT)" ]; then
        echo "$WWW_ROOT is not Empty, using existing structure."
    else
        echo "$WWW_ROOT is Empty, creating DerbyNet Website filesystem."
        cd /
        git clone https://github.com/jeffpiazza/derbynet.git && \
            mv /derbynet/website/* /usr/share/nginx/html/. && \
            mv /derbynet/templates/banner.inc /usr/share/nginx/html/inc/banner.inc && \
            rm -rf /derbynet && \
            test -d $WWW_ROOT/local || mkdir -m 777 $WWW_ROOT/local
    fi
else
	echo "Directory $DIR not found."
fi