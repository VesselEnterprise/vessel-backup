# Vessel Server

## Deployments

### How it works

Deployments are useful when the Vessel client needs to be installed on many machines. Deployments allow the client to obtain it's initial configuration using a deployment key.

When the Vessel client connects to the server for the first time with the deployment key, the username of the machine is automatically matched with an existing user in the Vessel database with the same username. The unique identifier of the Vessel user is returned to the client along with the client token. The client token allows the machine to connect to the Vessel API for logging, remote signing, heartbeat, and configuration updates.

Deployments can also have an expiration datetime so that client installation with a provided deployment key is only valid for the specified period of time. This is recommended for larger scale deployment scenarios where additional security may be required.

### Creating a new deployment

TODO

## Server Requirements

* Ubuntu Server 18.x
	* Download: https://www.ubuntu.com/download/server
* Apache HTTP Server (Recommended)
	* Download: https://httpd.apache.org/download.cgi
* MariaDB 10.x (Recommended)
	* Download: https://mariadb.org/download/
	* MySQL will also work, however, we support MariaDB!
* PHP 7.x
	* Download: https://secure.php.net/downloads.php
	* The following additional PHP packages are required:
		* php7.x-json
		* php7.x-mbstring
		* php7.x-xml
		* php7.x-sqlite3

**Please Note**: While many of these components may be installed on Windows, we do not support server configuration on Windows at this time
