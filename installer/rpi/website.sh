#! /bin/sh
. `dirname $0`/common.sh

if [ ! -d /var/www/html/derbynet -a -d ../../../website ] ; then
    cp -r ../../../website/ /var/www/html/derbynet
    # Alternatively: ln -s `realpath ../../../website` /var/www/html/derbynet

    # TODO Symbolic link to flash drive?
    mkdir -m 777 /var/www/html/derbynet/local
fi

######## Assumed packages
( dpkg -l | grep '^.i' | grep nginx > /dev/null) || \
    apt_get_install nginx

( dpkg -l | grep '^.i' | grep php5 > /dev/null) || \
    apt_get_install php5-fpm php5-curl php5-gd php5-cli php5-mysql php5-sqlite

######## Disable vanilla default site
rm -f /etc/nginx/sites-enabled/default 2>/dev/null

######## derbynet site

mkdir /etc/nginx/derbynet 2>/dev/null

if [ ! -f "/etc/nginx/derbynet/derbynet.key" ] ; then
    ssh-keygen -P "" -f "/etc/nginx/derbynet/derbynet.key"
    openssl req -new -subj "/C=US/ST=-/O=-/localityName=-/CN=$SERVERNAME/"\
            -key "/etc/nginx/derbynet/derbynet.key" \
            -out "/etc/nginx/derbynet/derbynet.csr"
    openssl x509 -req -days 3650 \
            -in "/etc/nginx/derbynet/derbynet.csr" \
            -signkey "/etc/nginx/derbynet/derbynet.key" \
            -out "/etc/nginx/derbynet/derbynet.pem"

    rm "/etc/nginx/derbynet/derbynet.csr"
fi

write_derbynet_snippet
cat <<EOF | sed -e "s/SERVERNAME/$SERVERNAME/" >/etc/nginx/sites-available/derbynet
server {
	listen 80;

	listen 443 ssl default_server;
	listen [::]:443 ssl default_server;
        # TODO
	# include snippets/snakeoil.conf;
	ssl_certificate     /etc/nginx/derbynet/derbynet.pem;
	ssl_certificate_key /etc/nginx/derbynet/derbynet.key;

	server_name SERVERNAME;

	include /etc/nginx/derbynet/server-snippet.conf;
}
EOF
ln -sf /etc/nginx/sites-available/derbynet /etc/nginx/sites-enabled

nginx -s reload
