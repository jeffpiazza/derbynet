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

# How large are the pictures you want to upload?  8M covers an 18-megapixel
# photo.
MAX_UPLOAD_SIZE=16M

#
# memory_limit = 128M is insufficient for photo_crop to rotate an 8M image; 256M
# is.  Not sure what's required for a full 16M image to rotate.
#
# The default session.gc_maxlifetime value can cause a session (including
# logged-in state of the user) to expire after about 24 minutes of inactivity --
# inconvenient for humans, and even more serious for headless scripts that don't
# re-login.
sed -e "s#^upload_max_filesize = [28]M#upload_max_filesize = $MAX_UPLOAD_SIZE#" \
    -e "s#^post_max_size = [2-8]M#post_max_size = $MAX_UPLOAD_SIZE#" \
    -e "s#^;date.timezone =#date.timezone = $TIMEZONE#" \
    -e "s#^memory_limit = 128M#memory_limit = 256M#" \
    -e "s#^;error_log = syslog#error_log = /var/log/php_errors#" \
    -e "s#^session.gc_maxlifetime = 1440#session.gc_maxlifetime = 28800#" \
    < $ORIGINAL_INI > /etc/php.ini

# Apache needs to be restarted for these changes to take effect; that happens in 
# the update-apache-config.sh script.
