# Vessel Enterprise Backup Client/Server

https://www.vesselenterprise.com/

Vessel Enterprise Backup Server is a secure, scalable, cloud-based or on-premise backup solution for the enterprise. The server is written in PHP and can live in the cloud (AWS, Google Cloud Platform, Azure, etc) or on-premise on your internal network. Vessel will feature a number of storage options including AWS S3, Azure Cloud Storage, Google Cloud Storage, and on-premise disk storage.

Vessel Enterprise Backup Client is a powerful cross-platform client written in C++ driven by the incredible Boost C++ library. (https://www.boost.org). The client features a direct sync to the Vessel Enterprise Server and the capability of uploading directly to 3rd party storage services such as AWS, Google, and Azure.

Vessel aims to be the first enterprise-grade open-source backup solution. We are looking for devs! If you are interested in helping, feel free to contact us to get involved in the project.

### Features (work in progress)

- Central Management server
 - Manage users, storage targets, files, machines, settings
 - Scalable to meet capacity needs
 - LDAP integration for users and computers
 - Manage user litigation hold
- Cross-platform desktop client
  - Quick and easy deploy using management server
  - Client check-in (sync user/machine data)
  - Automatic updates
  - Cloud or on-premise file backup
  - Multi part file uploads
- File download portal
  - Authorized end users can login to a web portal to download their backed up files
- Cloud Storage (AWS, Google, Azure)
- Network Bandwidth throttling for client application
- File compression
 - Decrease storage size and increase transfer speed
- File encryption
 - Configure specific users for encryption
 - Store files securely in the cloud or on your own file storage
- File versioning for specified filesize limits
- File upload weight/prioritization
- File type auto ignore
- User-level setting overrides and custom config
- File statistics and storage tracking
- Statistics and logging
