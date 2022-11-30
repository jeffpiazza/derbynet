# Docker Compose Deployment

This deployment method is intended to make the server portion perhaps a bit little easier.  It uses some of the work done by Jeff and Mitchell as well, it then expands on that work to hopefully make things a bit easier for those who have docker compose installed.

This docker-compose.yml file will spin up 2 docker containers and create a private docker network which is used by DerbyNet, the first container is nginx only and the other is a PHP 8.1 container running php-fpm.  Both containers are based on Alpine linux so everything is as tiny as possible and things spin up very fast.

    $docker image ls
    REPOSITORY                         TAG       IMAGE ID       CREATED          SIZE
    docker-compose-php-fpm             latest    a2c6297a3b06   29 seconds ago   44.2MB
    docker-compose-nginx               latest    6cd6f4214c94   35 seconds ago   39.1MB


The docker-compose.yml file will create 2 directories in the current working directory, the first directory is 'website' which will grab a clone of the derbynet git repo website directory and the second directory which gets created is 'data' which gets mounted into the php-fpm container as a writeable area as /data.

## TODO: Add comments.

## USAGE:

    git clone DERBYNET-REPO-URL
    cd debynet/installers/docker-compose
    docker compose up -d


To see the logs from both containers, you can simply run 'docker compose logs' and you'll see something like this:

    DerbyNet-nginx    | /docker-entrypoint.sh: /docker-entrypoint.d/ is not empty, will attempt to perform configuration
    DerbyNet-nginx    | /docker-entrypoint.sh: Looking for shell scripts in /docker-entrypoint.d/
    DerbyNet-nginx    | /docker-entrypoint.sh: Launching /docker-entrypoint.d/prep_filesystem.sh
    DerbyNet-nginx    | /usr/share/nginx/html is Empty, creating DerbyNet Website filesystem.
    DerbyNet-nginx    | Cloning into 'derbynet'...
    DerbyNet-nginx    | /docker-entrypoint.sh: Launching /docker-entrypoint.d/10-listen-on-ipv6-by-default.sh
    DerbyNet-nginx    | 10-listen-on-ipv6-by-default.sh: info: Getting the checksum of /etc/nginx/conf.d/default.conf
    DerbyNet-nginx    | 10-listen-on-ipv6-by-default.sh: info: /etc/nginx/conf.d/default.conf differs from the packaged version
    DerbyNet-nginx    | /docker-entrypoint.sh: Launching /docker-entrypoint.d/20-envsubst-on-templates.sh
    DerbyNet-nginx    | /docker-entrypoint.sh: Launching /docker-entrypoint.d/30-tune-worker-processes.sh
    DerbyNet-nginx    | /docker-entrypoint.sh: Configuration complete; ready for start up
    DerbyNet-nginx    | 2022/11/29 14:13:58 [notice] 1#1: using the "epoll" event method
    DerbyNet-nginx    | 2022/11/29 14:13:58 [notice] 1#1: nginx/1.22.1
    DerbyNet-nginx    | 2022/11/29 14:13:58 [notice] 1#1: built by gcc 11.2.1 20220219 (Alpine 11.2.1_git20220219) 
    DerbyNet-nginx    | 2022/11/29 14:13:58 [notice] 1#1: OS: Linux 5.14.0-70.30.1.el9_0.x86_64
    DerbyNet-nginx    | 2022/11/29 14:13:58 [notice] 1#1: getrlimit(RLIMIT_NOFILE): 1073741816:1073741816
    DerbyNet-nginx    | 2022/11/29 14:13:58 [notice] 1#1: start worker processes
    DerbyNet-nginx    | 2022/11/29 14:13:58 [notice] 1#1: start worker process 45
    DerbyNet-nginx    | 2022/11/29 14:13:58 [notice] 1#1: start worker process 46
    DerbyNet-nginx    | 2022/11/29 14:13:58 [notice] 1#1: start worker process 47
    DerbyNet-nginx    | 2022/11/29 14:13:58 [notice] 1#1: start worker process 48
    DerbyNet-nginx    | 192.168.1.16 - - [29/Nov/2022:14:18:53 +0000] "GET / HTTP/1.1" 302 5 "-" "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/107.0.0.0 Safari/537.36" "-"
    DerbyNet-Backend  | 192.168.240.3 -  29/Nov/2022:14:18:53 +0000 "GET /index.php" 302
    DerbyNet-Backend  | 192.168.240.3 -  29/Nov/2022:14:18:53 +0000 "GET /setup.php" 200
    DerbyNet-nginx    | 192.168.1.16 - - [29/Nov/2022:14:18:53 +0000] "GET /setup.php HTTP/1.1" 200 13323 "-" "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/107.0.0.0 Safari/537.36" "-"
    DerbyNet-nginx    | 192.168.1.16 - - [29/Nov/2022:14:18:53 +0000] "GET /css/jquery-ui.min.css HTTP/1.1" 200 32076 "http://192.168.1.200:8077/setup.php" "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/107.0.0.0 Safari/537.36" "-"
    DerbyNet-nginx    | 192.168.1.16 - - [29/Nov/2022:14:18:53 +0000] "GET /css/mobile.css HTTP/1.1" 200 18295 "http://192.168.1.200:8077/setup.php" "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/107.0.0.0 Safari/537.36" "-"
    DerbyNet-nginx    | 192.168.1.16 - - [29/Nov/2022:14:18:53 +0000] "GET /css/global.css HTTP/1.1" 200 5387 "http://192.168.1.200:8077/setup.php" "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/107.0.0.0 Safari/537.36" "-"
