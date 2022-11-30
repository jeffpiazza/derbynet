FROM alpine:3.17

#Setup some variables we'll be using
ARG TZ
ARG MAX_UPLOAD_SIZE

#Install all of the dependancies
RUN apk add libcurl bash php81-fpm php81-curl php81-gd php81-sqlite3 php81-odbc php81-session php81-pdo php81-pdo_sqlite

#Send the logs to someplace docker can use them
RUN echo -e "access.log = /proc/self/fd/2" >> /etc/php81/php-fpm.conf

#Set the timezone because PHP has liked it that way since PHP5
RUN ln -snf /usr/share/zoneinfo/$TZ /etc/localtime && echo $TZ > /etc/timezone

#Set some PHP.ini Vars
RUN sed -i "s/upload_max_filesize = 2M/upload_max_filesize = ${MAX_UPLOAD_SIZE}/g" /etc/php81/php.ini
RUN sed -i "s/post_max_size = 8M/post_max_size = ${MAX_UPLOAD_SIZE}/g" /etc/php81/php.ini
RUN sed -i "s/memory_limit = 128M/memory_limit = 256M/g" /etc/php81/php.ini
RUN sed -i "s/session.gc_maxlifetime = 1440/session.gc_maxlifetime = 28800/g" /etc/php81/php.ini

#Fix some php-fpm settings so they work with this application
RUN sed -i "s/listen = 127.0.0.1:9000/listen = 0.0.0.0:9000/g" /etc/php81/php-fpm.d/www.conf
RUN sed -i "s/;chdir = \/var\/www/chdir = \/usr\/share\/nginx\/html/g" /etc/php81/php-fpm.d/www.conf

#Set some permissions on some directories
RUN test -d /data || mkdir -m 777 /data
RUN chown -R nobody:nobody /data
RUN chmod -R 777 /data

#Fix the permisions on /data and start the php-fpm process 
CMD chown -R nobody:nobody /data; /usr/sbin/php-fpm81 -F

#Expose port 9000
EXPOSE 9000