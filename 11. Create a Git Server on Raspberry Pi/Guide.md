#Install gogs On Your Raspberry Pi

##Prerequisite

####A. Prepare a running Raspberry Pi

You need to install Raspbian on your Raspberry Pi
[Download Link](http://downloads.raspberrypi.org/raspbian_latest)

####B. Install Golang

You can install golang from source code with this script.

It checks out latest code from the repository, build, and install binaries for Raspberry Pi.

It will take about 1 hour or so.

After installation, add following to your rc file(eg. .zshrc, .bashrc, …):
```bash
# for go
if [ -d /opt/go/bin ]; then
  export GOROOT=/opt/go
  export GOPATH=$HOME/srcs/go
  export PATH=$PATH:$GOROOT/bin
fi
```
Close your terminal and open a new one, then you will be able to run golang excutables.

####C. Install Database Software for Gogs

You’ll need to install some more packages before going to the next step.
```bash
# Gogs supports MySQL, PostgreSQL, and SQLite. You can choose one.
$ sudo apt-get install mysql-server
# or
$ sudo apt-get install postgresql
```
##Checkout and build Gogs source code

Gogs has no pre-built binaries for Raspberry Pi yet, so there’s only one choice: Install From Source Code.

####a. If you don’t need SQLite3 support,

In your terminal,
```bash
# download and install dependencies
$ go get -u github.com/gogits/gogs

# build
$ cd $GOPATH/src/github.com/gogits/gogs
$ go build
```
####b. If you need to enable SQLite3,
```bash
# download and install dependencies
$ go get -u -tags sqlite github.com/gogits/gogs

# build
$ cd $GOPATH/src/github.com/gogits/gogs
$ go build -tags sqlite
```

##Configure Gogs

####a. Setup database (MySQL)

Create a MySQL database for Gogs.
```bash
$ mysql -uroot -p
Grant some priviliges to the created database.

mysql> DROP DATABASE IF EXISTS gogs;
mysql> CREATE DATABASE IF NOT EXISTS gogs CHARACTER SET utf8 COLLATE utf8_general_ci;
mysql> GRANT ALL ON gogs.* to 'gogs'@'localhost' identified by 'SOME_PASSWORD';
```
####b. Setup repository

Create a directory where your raw repository data will be saved.
```bash
$ mkdir -p ~/somewhere/gogs-repositories
```
####c. Edit configuration

Gogs needs some configuration before running.

It recommends(or forces? I’m not sure.) to keep custom config files rather than modifying the default ones directly.

So, duplicate the conf/app.ini file first.
```bash
$ cd $GOPATH/src/github.com/gogits/gogs

$ mkdir -p custom/conf
$ cp conf/app.ini custom/conf/app.ini
```
Now let’s edit the duplicated config file.
```bash
$ vi custom/conf/app.ini
```
#####(1) repository path

Change ROOT to the path of the directory which you created above.

[repository]
```ini
ROOT = /home/pi/somewhere/gogs-repositories
```
#####(2) database connection

Change username and password for database connection.
```ini
[database]
; Either "mysql", "postgres" or "sqlite3", it's your choice
DB_TYPE = mysql
HOST = 127.0.0.1:3306
NAME = gogs
USER = gogs
PASSWD = SOME_PASSWORD
; For "postgres" only, either "disable", "require" or "verify-full"
SSL_MODE = disable
; For "sqlite3" only
PATH = data/gogs.db
```
#####(3) run mode

If you want Gogs to be run in production mode, change it.
```ini
; Either "dev", "prod" or "test", default is "dev"
RUN_MODE = prod
```
#####(4) server configuration

Change domain, port, or other values.
```ini
[server]
PROTOCOL = http
DOMAIN = my-raspberrypi-domain.com
ROOT_URL = %(PROTOCOL)s://%(DOMAIN)s:%(HTTP_PORT)s/
HTTP_ADDR =
HTTP_PORT = 3000
SSH_PORT = 22
; Disable CDN even in "prod" mode
OFFLINE_MODE = false
DISABLE_ROUTER_LOG = false
; Generate steps:
; $ cd path/to/gogs/custom/https
; $ go run $GOROOT/src/pkg/crypto/tls/generate_cert.go -ca=true -duration=8760h0m0s -host=my-raspberrypi-domain.com
CERT_FILE = custom/https/cert.pem
KEY_FILE = custom/https/key.pem
; Upper level of template and static file path
; default is the path where Gogs is executed
STATIC_ROOT_PATH =
```
You may also have to generate certificate files as the comment says:
```bash
$ cd $GOPATH/src/github.com/gogits/gogs
$ mkdir -p custom/https
$ cd custom/https
$ go run $GOROOT/src/pkg/crypto/tls/generate_cert.go -ca=true -duration=8760h0m0s -host=my-raspberrypi-domain.com
```
#####(5) secret key

Change the default secret key for security.
```ini
[security]
; !!CHANGE THIS TO KEEP YOUR USER DATA SAFE!!
SECRET_KEY = _secret_0123456789_
```
#####(6) app name

If you want to change the page title, you can change this value
```ini
; App name that shows on every page title
APP_NAME = Gogs on my Raspberry Pi
```

##Run Gogs

####A. Run in the shell

Gogs can be started from the shell.
```bash
$ cd $GOPATH/src/github.com/gogits/gogs
$ ./gogs web
```

##Install Gogs

####A. Open your browser

Open your browser, like Chrome, Firefox, Safari......
But don't use IE :P

####B. Install Gogs
Go to http://Your_Raspberry_Pi_Network_Location:Your Port
Like me, I didn't change port, and my RPi IP is 192.168.1.10
http://192.168.1.10:3000

####C. Setting
You can change some config in there.

##Enjoy your Gogs Server!!!
