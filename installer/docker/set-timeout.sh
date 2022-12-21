#! /bin/bash
#
# Adjusts max_execution_time for PHP-fpm and fastcgi_read_timeout for nginx.
# Assumes php-fpm is running under supervisord.
#
# Takes an optional argument for the new timeout in seconds, otherwise assumes 600.
#
# From outside the docker image, get the image name with ( docker ps ), then:
#    docker exec ((docker image name))  set-timeout.sh
# or
#    docker exec ((docker image name))  set-timeout.sh 900
#

TIMEOUT=600

if [ "$#" -gt 0 ] ; then
    TIMEOUT="$1"
    shift
fi

for INI in `find /etc/php* -name php.ini` ; do
    sed -i \
        -e "s/^ *max_execution_time *= *[0-9][0-9]*/max_execution_time = $TIMEOUT/" \
        $INI
done


sed -i \
    -e "/fastcgi_read_timeout/ s/.*/fastcgi_read_timeout $TIMEOUT;/" \
    /etc/nginx/http.d/default.conf
nginx -s reload
