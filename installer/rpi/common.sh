#! /bin/sh

SERVERNAME=derby.net

APT_UPDATED=0
apt_get_install() {
    if [ "$APT_UPDATE" -eq 0 ] ; then
	apt-get update
	APT_UPDATED=1
    fi
    apt-get install $@
}

write_derbynet_snippet() {
    cat <<EOF >/etc/nginx/derbynet/server-snippet.conf
  root /var/www/html/derbynet;

  index index.php;

  location ~ \.php(/.*)?$ {
    include snippets/fastcgi-php.conf;
    fastcgi_pass unix:/var/run/php5-fpm.sock;
  }
EOF
}
