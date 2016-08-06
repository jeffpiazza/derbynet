#! /bin/sh

SERVERNAME=derby.net
MAX_UPLOAD_SIZE=8M

APT_UPDATED=0
apt_get_install() {
    if [ "$APT_UPDATE" -eq 0 ] ; then
	apt-get update
	APT_UPDATED=1
    fi
    apt-get install $@
}

write_derbynet_snippet() {
    cat <<EOF | sed -e "s/MAX_UPLOAD_SIZE/$MAX_UPLOAD_SIZE/" >/etc/nginx/derbynet/server-snippet.conf
  root /var/www/html/derbynet;

  index index.php;

  client_max_body_size MAX_UPLOAD_SIZE;

  location ~ \.php(/.*)?$ {
    include snippets/fastcgi-php.conf;
    fastcgi_pass unix:/var/run/php5-fpm.sock;
  }
EOF
}
