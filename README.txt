Setting up on a Mac:

(1) To get MySQL and PHP running, I suggest following one of these tutorials:
http://coolestguidesontheplanet.com/get-apache-mysql-php-phpmyadmin-working-osx-10-9-mavericks/
http://akrabat.com/computing/setting-up-php-mysql-on-os-x-mavericks/.

(1a) http://clickontyler.com/web-sharing/ offers a nice System Preferences plug-in that lets you turn the web server ("web sharing") on and off.

(2) Create a mysql database and database user for web race manager.  This example creates database webracemanager with user racemanageruser and password mypassword:

mysql -u root -p
Enter password:

mysql> create database webracemanager;
Query OK, 1 row affected (0.08 sec)

mysql> create user 'racemanageruser'@'%' identified by 'mypassword';
Query OK, 0 rows affected (0.21 sec)

mysql> grant all privileges on webracemanager.* to 'racemanageruser'@'%';
Query OK, 0 rows affected (0.00 sec)

mysql> flush privileges;
Query OK, 0 rows affected (0.38 sec)

mysql> exit

(3) Copy the contents of the webserver directory to somewhere under the Apache document root, /Library/WebServer/Documents/.  

If you want to serve web race manager as the only thing on this web server (i.e., if you don't have any other web content being served from this Mac) you would enter this command in the Terminal app

sudo mv -R website/* /Library/WebServer/Documents/

and confirm that it worked by opening a browser and pointing it to http://localhost.

[ For the remainder of these instructions, we'll assume that you installed web race manager at root level, as above.  Modify commands as appropriate if, instead, you installed in a subdirectory. ]

(4) Create a "local" subdirectory if one doesn't already exist, and make sure it's writable by the _www user, e.g.,

sudo mkdir /Library/WebServer/Documents/local
sudo chmod 777 /Library/WebServer/Documents/local

(5) Visit http://localhost, which should redirect to http://localhost/setup.php.  When you click the Configure button, you need to enter an ODBC connection string, plus ODBC user name and password, e.g:

Connection string: mysql:host=localhost;dbname=webracemanager
User name: racemanageruser
Password: mypassword

(6) Click "Initialize Schema".  Despite the warnings, have no fears about initializing an empty database!

(7) If desired, edit the file config-roles.inc that's now present in the local subdirectory (i.e., /Library/WebServer/Documents/local/config-roles.inc) to change the roles, passwords, or permissions:

open -a TextEdit /Library/WebServer/Documents/sandbox/local/config-roles.inc

(8) It's a good practice to remove write permissions from your 'local' subdirectory after initialization:

sudo chmod -R 755 /Library/WebServer/Documents/local

(9) If your timer is connected to the same Mac, run the timer manager from the command line:

java -jar derby-timer.jar http://localhost

which will attempt to detect any connected timer devices.  (The timer manager will probe continuously until a timer is detected.)
