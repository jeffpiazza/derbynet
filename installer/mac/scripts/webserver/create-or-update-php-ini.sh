#! /bin/sh

# If /etc/php.ini is absent, manufacture one from /etc/php.ini.default

ORIGINAL_INI=/etc/php.ini.default

if [ -f /etc/php.ini ] ; then
    mv /etc/php.ini /etc/php.ini.pre-derbynet
    ORIGINAL_INI=/etc/php.ini.pre-derbynet
fi

touch     /var/log/php_errors
chmod 777 /var/log/php_errors

TIMEZONE=`systemsetup -gettimezone | sed -e "s/Time Zone: //"`

sed -e "s#^upload_max_filesize = 2M#upload_max_filesize = 8M#" \
    -e "s#^;date.timezone =#date.timezone = $TIMEZONE#" \
    -e "s#^;error_log = syslog#error_log = /var/log/php_errors#" \
    < $ORIGINAL_INI > /etc/php.ini

# Apache needs to be restarted for these changes to take effect; that happens in 
# the update-apache-config.sh script.
