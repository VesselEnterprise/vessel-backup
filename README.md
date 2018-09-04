# Vessel Enterprise Backup Platform

Visit our site:
https://www.vesselenterprise.com/

Development: IN PROGRESS  
Release: TBD

- [Vessel Enterprise Backup Platform](#vessel-enterprise-backup-platform)
  * [What is Vessel?](#what-is-vessel-)
  * [Features](#features)
    + [Client](#client)
    + [Server](#server)
  * [Security](#security)
  * [Management Portal](#management-portal)
  * [Managed Services and Setup](#managed-services-and-setup)
  * [Documentation](#documentation)
  * [Technology stack](#technology-stack)
    + [Client](#client-1)
    + [Server](#server-1)
  * [Contributing](#contributing)

## What is Vessel?

Vessel is an open-source enterprise platform for file backup and storage management. Vessel supports all major cloud storage providers including Amazon, Azure, and Google. You can also use your own on-premise NFS storage provider with the platform.

## Features

### Client
* Builds: Cross-platform (Linux, Windows) (OSX TBD)
* Storage: Support for AWS, Azure, Google, NFS; File Encryption
* Network: Bandwidth throttling and uncap
* Files: File extension bypass, weighted file priority, security issue detection
* Deployment: Clients can obtain a secure token to communicate with the platform when bootstrapping with a deployment key
* Heartbeat: Clients check-in with the platform regularly and upload statistics, logging, and other information
* Encryption rules: Use Cloud Encryption by specifying a rule set to protect your CEO's files
* Auto-updater: Clients can be configured to keep their application up-to-date with the most recent compatible platform version

### Server
* Management Portal
* Files: Restore, Manage, Reports
* Storage Providers: Add your preferred cloud storage provider to the platform. Clients will automatically use the provider for file backup
* Users: Authentication framework, LDAP integration
* Clients: View statistics and settings for machines that communicate with the platform
* Settings: Manage platform and user specific settings
* Reporting: File statistics, Network utilization, Backup %, etc
* Tools: Security issue monitoring,
* Monitoring: Error logging and email alerts

## Security

Vessel uses remote signing methods for cloud providers so that IAM user secrets are never exposed to the client. Vessel clients are issued secure tokens to ensure secure communication with the Vessel API.

## Management Portal

Manage organization configuration, storage providers, files, and more from the portal. Create deployments to deploy the client to your user base,

[[vessel-backup/etc/vessel_ui_example.png|alt=Vessel Management Portal]]

## Managed Services and Setup

Vessel will offer managed services and setup for organizations looking to deploy the platform. More details will be available upon release.

## Documentation

Documentation is currently being built. More details will come soon.

## Technology stack

### Client
* C++, Boost, RapidJSON, SQLite, CryptoPP

### Server
* PHP, Laravel, MariaDB (recommended)
* Semantic UI

## Contributing

If you are interested in helping out with this project, please contact me!
