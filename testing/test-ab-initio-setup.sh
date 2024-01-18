#! /bin/sh

# On Mac under Docker, the expected set-up is that the /var/lib/derbynet
# directory is mapped to ~/Public/DerbyNet, and
# ~/Public/DerbyNet/config-database.inc and config-roles.inc are found there.
# The db_connection_string in config-database.inc points to a directory under
# /var/lib/derbynet that needs to be mapped to the corresponding directory under
# ~/Public/DerbyNet.
#
# Testing strategy would be:
#  Remove config-database.inc temporarily.

# This only works under strong assumptions about where DERBYNET_CONFIG_DIR
# points; those assumptions are no longer true.
##############################
# Performs some basic tests on a newly-installed or partially-installed website.
#
# For a remote server, we wouldn't have access to the 'local' directory, so this
# test only runs for localhost
#
# Worse, since we want to manipulate the directory directly, without going
# through the web server, we have to know or assume what the directory structure
# is.

BASE_URL=$1
set -e -E -o pipefail
source `dirname $0`/common.sh

manipulate_local_directory() {
    # Clean up results from a failed previous attempt, if any
    if [ -d $WEBDIR/xlocalx ] ; then
        tput setaf 2  # green text
        echo Recovering xlocalx directory
        tput setaf 0  # black text

        [ -d $WEBDIR/local ] && rm -rf $WEBDIR/local
        mv $WEBDIR/xlocalx $WEBDIR/local
    fi

    # $WEBDIR/local
    echo Performing ab initio set-up testing on $WEBDIR/local

    ## ----------------------------------
    echo '   ' With no local directory...
    mv $WEBDIR/local $WEBDIR/xlocalx

    # Redirects to set-up page
    curl_get index.php | expect_one 'You need to create'
    
    ## ----------------------------------
    echo '   ' With unwritable directory
    mkdir -m 0555 $WEBDIR/local
    curl_get index.php | expect_one "but isn't writable"

    ## ----------------------------------
    # echo '   ' With writable empty directory but no default path
    chmod 0777 $WEBDIR/local
}

if [ "$BASE_URL" = "localhost" ]; then
    if [ -d ~/Public/DerbyNet/ ]; then
        CONFIGDIR=~/Public/DerbyNet
        DATADIR=~/Public/DerbyNet
    elif [ -d /Library/WebServer/Documents/derbynet/ ]; then
        # Mac
        WEBDIR=/Library/WebServer/Documents/derbynet
        CONFIGDIR=$WEBDIR/local
        DATADIR=~/Public/DerbyNet
    elif [ -d /var/www/html/derbynet/ ]; then
        # Debian/Linux
        WEBDIR=/var/www/html/derbynet
        CONFIGDIR=$WEBDIR/local
        DATADIR=/var/lib/derbynet
    else
        tput setaf 1  # red text
        echo Host system type not recognized
        tput setaf 0  # black text
    fi

    YEAR=`date +%Y`
    [ -d $DATADIR ] || mkdir -m 777 $DATADIR
    [ -d $DATADIR/$YEAR/this-will-succeed ] && rm -rf $DATADIR/$YEAR/this-will-succeed

    if [ -n "$WEBDIR" ]; then
        manipulate_local_directory
    fi

    [ -f $CONFIGDIR/config-database.inc ] && \
        mv $CONFIGDIR/config-database.inc $CONFIGDIR/xconfig-database.inc
    [ -f $CONFIGDIR/config-roles.inc ] && \
        mv $CONFIGDIR/config-roles.inc $CONFIGDIR/xconfig-roles.inc

    restore_configs() {
        echo Restoring configs
        [ -f $CONFIGDIR/xconfig-database.inc ] && \
            mv $CONFIGDIR/xconfig-database.inc $CONFIGDIR/config-database.inc
        [ -f $CONFIGDIR/xconfig-roles.inc ] && \
            mv $CONFIGDIR/xconfig-roles.inc $CONFIGDIR/config-roles.inc
    }
    trap 'restore_configs' 1 2 3 4 15
    
    ## ----------------------------------
    echo '   ' Successful set-up

    sleep 1

    
    curl_get index.php | expect_one 'configure the database first'
    curl_postj action.php "action=setup.nodata&ez-new=this-will-succeed" | check_jsuccess

    # confirm config-database and config-roles
    [ -f $CONFIGDIR/config-database.inc ] || test_fails Missing database config
    [ -f $CONFIGDIR/config-roles.inc ] || test_fails Missing roles config

    # confirm sqlite database file and directories
    [ -f $DATADIR/$YEAR/this-will-succeed/derbynet.sqlite3 ] || test_fails Database not created
    [ -d $DATADIR/$YEAR/this-will-succeed/cars ] || test_fails Car photo directory not created
    [ -d $DATADIR/$YEAR/this-will-succeed/racers ] || test_fails Racer photo directory not created
    [ -d $DATADIR/$YEAR/this-will-succeed/videos ] || test_fails Video directory not created

    ## ----------------------------------
    echo '   ' Removing database file
    rm -rf $DATADIR/$YEAR/this-will-succeed
    
    curl_get index.php | expect_one 'a problem opening the database'

    restore_configs    

    ## ----------------------------------
    if [ -n "$WEBDIR" ] ; then
        echo '   ' Cleaning up
        rm -rf $WEBDIR/local
        mv $WEBDIR/xlocalx $WEBDIR/local
    fi
else
    tput setaf 2  # green text
    echo Not testing ab initio set-up
    tput setaf 0  # black text
fi
