FROM alpine:3.17
ARG TZ
ARG MAX_UPLOAD_SIZE
RUN apk add libcurl bash php81-fpm php81-curl php81-gd php81-sqlite3 php81-odbc php81-session php81-pdo php81-pdo_sqlite
RUN echo -e "access.log = /proc/self/fd/2" >> /etc/php81/php-fpm.conf
RUN ln -snf /usr/share/zoneinfo/$TZ /etc/localtime && echo $TZ > /etc/timezone
RUN sed -i "s/upload_max_filesize = 2M/upload_max_filesize = ${MAX_UPLOAD_SIZE}/g" /etc/php81/php.ini
RUN sed -i "s/post_max_size = 8M/post_max_size = ${MAX_UPLOAD_SIZE}/g" /etc/php81/php.ini
RUN sed -i "s/memory_limit = 128M/memory_limit = 256M/g" /etc/php81/php.ini
RUN sed -i "s/session.gc_maxlifetime = 1440/session.gc_maxlifetime = 28800/g" /etc/php81/php.ini
RUN sed -i "s/listen = 127.0.0.1:9000/listen = 0.0.0.0:9000/g" /etc/php81/php-fpm.d/www.conf
RUN sed -i "s/;chdir = \/var\/www/chdir = \/usr\/share\/nginx\/html/g" /etc/php81/php-fpm.d/www.conf
RUN test -d /data || mkdir -m 777 /data
RUN chown -R nobody:nobody /data
RUN chmod -R 777 /data
CMD chown -R nobody:nobody /data; /usr/sbin/php-fpm81 -F
EXPOSE 9000