-- phpMyAdmin SQL Dump
-- version 4.6.6deb5
-- https://www.phpmyadmin.net/
--
-- Host: localhost:3306
-- Generation Time: Mar 17, 2018 at 10:41 AM
-- Server version: 10.1.25-MariaDB-1
-- PHP Version: 7.1.8-1ubuntu1

SET SQL_MODE = "NO_AUTO_VALUE_ON_ZERO";
SET time_zone = "+00:00";


/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET @OLD_CHARACTER_SET_RESULTS=@@CHARACTER_SET_RESULTS */;
/*!40101 SET @OLD_COLLATION_CONNECTION=@@COLLATION_CONNECTION */;
/*!40101 SET NAMES utf8mb4 */;

--
-- Database: `backup`
--

-- --------------------------------------------------------

--
-- Table structure for table `backup_api_session`
--

CREATE TABLE `backup_api_session` (
  `session_id` int(11) UNSIGNED NOT NULL,
  `user_id` int(11) NOT NULL,
  `endpoint` varchar(32) DEFAULT NULL,
  `http_method` varchar(32) NOT NULL,
  `ip_address` varchar(32) DEFAULT NULL,
  `created` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

-- --------------------------------------------------------

--
-- Table structure for table `backup_client`
--

CREATE TABLE `backup_client` (
  `client_id` int(11) NOT NULL,
  `file_name` varchar(128) NOT NULL,
  `version` varchar(32) NOT NULL,
  `uploaded` datetime NOT NULL DEFAULT CURRENT_TIMESTAMP
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

-- --------------------------------------------------------

--
-- Table structure for table `backup_client_setting`
--

CREATE TABLE `backup_client_setting` (
  `setting_id` int(11) NOT NULL,
  `name` varchar(32) NOT NULL,
  `value` varchar(512) NOT NULL,
  `description` varchar(512) NOT NULL,
  `data_type` varchar(32) NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

--
-- Dumping data for table `backup_client_setting`
--

INSERT INTO `backup_client_setting` (`setting_id`, `name`, `value`, `description`, `data_type`) VALUES
(1, 'max_transfer_speed', '102400', 'Maximum transfer/upload speed (bytes) per second', 'int'),
(2, 'compression_level', '6', 'Default ZLIB compression level (0-9)', 'int'),
(3, 'compress_transfer', '1', 'Always compress file before transferring (0 or 1)', 'int'),
(4, 'skip_period_dirs', '1', 'Do not read directories that start with a period. (Eg. \".config\" ) (0 or 1)', 'int'),
(5, 'new_files_first', '1', 'Always backup recently modified files before older files (0 or 1)', 'int'),
(6, 'multipart_filesize', '104857600', 'Files are uploaded in chunks when a file is greater than or equal to this filesize (bytes)', 'int'),
(7, 'master_server', 'http://10.1.10.208', 'The master server which the backup client connects to', 'string');

-- --------------------------------------------------------

--
-- Table structure for table `backup_file`
--

CREATE TABLE `backup_file` (
  `file_id` varchar(40) NOT NULL COMMENT 'sha-1 hash of the full path to the file',
  `file_name` varchar(255) NOT NULL,
  `file_size` bigint(20) UNSIGNED NOT NULL,
  `user_id` int(11) NOT NULL,
  `file_type` varchar(16) NOT NULL,
  `hash` varchar(40) NOT NULL COMMENT 'sha-1 hash of the file contents',
  `file_path` varchar(260) NOT NULL,
  `uploaded` tinyint(1) NOT NULL,
  `encrypted` tinyint(1) NOT NULL DEFAULT '0',
  `compressed` tinyint(1) NOT NULL DEFAULT '0',
  `last_modified` timestamp NULL DEFAULT NULL,
  `last_backup` timestamp NULL DEFAULT NULL,
  `created` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

-- --------------------------------------------------------

--
-- Table structure for table `backup_ignore_type`
--

CREATE TABLE `backup_ignore_type` (
  `type_id` int(10) UNSIGNED NOT NULL,
  `file_type` varchar(32) NOT NULL,
  `active` tinyint(1) NOT NULL DEFAULT '1',
  `updated` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

--
-- Dumping data for table `backup_ignore_type`
--

INSERT INTO `backup_ignore_type` (`type_id`, `file_type`, `active`, `updated`) VALUES
(1, '.mp3', 1, '2018-02-10 20:08:37'),
(2, '.wma', 1, '2018-02-10 20:08:37'),
(3, '.exe', 1, '2018-02-10 20:08:37'),
(4, '.mpg', 1, '2018-02-10 20:08:37'),
(5, '.wav', 1, '2018-02-10 20:08:37'),
(6, '.mpeg', 1, '2018-02-10 20:08:37'),
(7, '.mp4', 1, '2018-02-10 20:08:37'),
(8, '.mp2', 1, '2018-02-10 20:08:37'),
(9, '.avi', 1, '2018-02-10 20:08:37'),
(10, '.bat', 1, '2018-02-10 20:08:37'),
(11, '.cmd', 1, '2018-02-10 20:09:42'),
(12, '.ini', 1, '2018-02-10 20:09:42'),
(13, '.avi', 1, '2018-02-10 20:09:42'),
(14, '.mov', 1, '2018-02-10 20:09:42'),
(15, '.m3u', 1, '2018-02-10 20:09:42'),
(16, '.m4a', 1, '2018-02-10 20:09:42'),
(17, '.pbf', 1, '2018-02-10 20:09:42'),
(18, '.msi', 1, '2018-02-10 20:09:42'),
(19, '.m4v', 1, '2018-02-10 20:09:42'),
(20, '.7z', 1, '2018-02-10 20:09:42'),
(21, '.m4p', 1, '2018-02-10 20:10:25'),
(22, '.itl', 1, '2018-02-10 20:10:25'),
(23, '.itdb', 1, '2018-02-10 20:10:25'),
(24, '.cab', 1, '2018-02-10 20:10:25'),
(25, '.wmv', 1, '2018-02-10 20:10:25'),
(26, '.dll', 1, '2018-02-10 20:11:30'),
(27, '.tmp', 1, '2018-02-10 20:11:30'),
(28, '.vob', 1, '2018-02-10 20:11:30'),
(29, '.mkv', 1, '2018-02-10 20:11:30'),
(30, '.nds', 1, '2018-02-10 20:11:30'),
(31, '.flac', 1, '2018-02-10 20:11:30'),
(32, '.iso', 1, '2018-02-10 20:11:30'),
(33, '.inf', 1, '2018-02-10 20:11:30'),
(34, '.vmdk', 1, '2018-02-10 20:11:30'),
(35, '.ost', 1, '2018-02-10 20:11:30');

-- --------------------------------------------------------

--
-- Table structure for table `backup_log`
--

CREATE TABLE `backup_log` (
  `entry_id` bigint(10) UNSIGNED NOT NULL,
  `message` text NOT NULL,
  `type` varchar(32) NOT NULL,
  `user_id` int(11) NOT NULL,
  `priority` int(11) NOT NULL,
  `error` tinyint(1) NOT NULL DEFAULT '0',
  `logged_at` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

-- --------------------------------------------------------

--
-- Table structure for table `backup_machine`
--

CREATE TABLE `backup_machine` (
  `machine_id` int(11) NOT NULL,
  `name` varchar(64) NOT NULL,
  `os` varchar(100) DEFAULT NULL,
  `dns_name` varchar(100) DEFAULT NULL,
  `ip_address` varchar(32) DEFAULT NULL,
  `domain` varchar(100) DEFAULT NULL,
  `client_version` varchar(32) DEFAULT NULL,
  `last_check_in` timestamp NULL DEFAULT NULL,
  `last_backup` timestamp NULL DEFAULT NULL,
  `source` varchar(32) DEFAULT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

-- --------------------------------------------------------

--
-- Table structure for table `backup_refresh_token`
--

CREATE TABLE `backup_refresh_token` (
  `user_id` int(10) UNSIGNED NOT NULL,
  `refresh_token` varchar(255) NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

-- --------------------------------------------------------

--
-- Table structure for table `backup_role`
--

CREATE TABLE `backup_role` (
  `role_id` int(11) UNSIGNED NOT NULL,
  `name` varchar(32) NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

--
-- Dumping data for table `backup_role`
--

INSERT INTO `backup_role` (`role_id`, `name`) VALUES
(1, 'admin'),
(2, 'reporter');

-- --------------------------------------------------------

--
-- Table structure for table `backup_setting`
--

CREATE TABLE `backup_setting` (
  `setting_id` int(11) NOT NULL,
  `name` varchar(32) NOT NULL,
  `display_name` varchar(100) NOT NULL,
  `value` varchar(255) NOT NULL,
  `description` text NOT NULL,
  `active` tinyint(1) NOT NULL DEFAULT '1',
  `type` varchar(32) NOT NULL,
  `updated` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

--
-- Dumping data for table `backup_setting`
--

INSERT INTO `backup_setting` (`setting_id`, `name`, `display_name`, `value`, `description`, `active`, `type`, `updated`) VALUES
(1, 'upload_use_blob', 'Use BLOB for Temporary Chunk Storage', '0', 'Use database BLOBs for temporary file storage', 1, 'Database', '2018-02-24 16:13:57'),
(2, 'temp_storage_path', 'Temporary Storage Path', '', 'Filesystem path where temporary file upload data should be stored.', 1, 'Upload', '2018-02-24 16:14:06'),
(3, 'ldap_server', 'LDAP Server', '', 'LDAP server used for importing users and computers', 1, 'LDAP', '2018-03-17 14:38:58'),
(4, 'ldap_user', 'LDAP Username', '', 'LDAP username used for connecting to LDAP server', 1, 'LDAP', '2018-03-17 14:39:01'),
(5, 'ldap_port', 'LDAP Port', '', 'Port used to connect to the LDAP server', 1, 'LDAP', '2018-03-17 14:39:04'),
(6, 'ldap_pwd', 'LDAP Password', '', 'LDAP user password used to connect to LDAP server', 1, 'LDAP', '2018-03-17 14:39:06'),
(7, 'ldap_user_tree', 'LDAP User DN', '', 'Base DN of where to search for LDAP user objects', 1, 'LDAP', '2018-03-17 14:39:09'),
(8, 'ldap_pc_tree', 'LDAP Computer DN', '', 'Base DN of where to search for LDAP computers', 1, 'LDAP', '2018-03-17 14:39:13'),
(10, 'ldap_activate_code', 'LDAP User Activation Code', 'default', 'Users who are imported via LDAP will all use the same activation code. When the client checks into the server for the first time, they will be able to activate using the default activation code. It is recommended to NOT change this value.', 1, 'LDAP', '2018-03-02 02:28:38'),
(11, 'encrypt_all', 'Encrypt File Uploads', '0', 'Encrypt all file uploads', 1, 'Security', '2018-02-24 16:14:25'),
(12, 'default_target', 'Default Upload Target ID', '1', 'The target_id of the default file upload target. This will apply to all users by default, but individual targets can be set per-user', 1, 'File Upload', '2018-02-24 16:14:25'),
(13, 'token_expiry', 'Token Expiration Hours', '72', 'Number of hours in which a user\'s access token expires and must be refreshed', 1, 'Security', '2018-02-24 16:14:25'),
(14, 'store_compressed', 'Store Upload Files Compressed', 'true', 'Set this flag to true if to store uploaded files as compressed on the server. File names are appended with \".gz\" when compressed', 1, 'Storage', '2018-02-24 16:14:25'),
(15, 'file_version_size', 'Maximum file size for file versioning', '5242880', 'Maximum file size used for file versioning. Files greater than this size will not be versioned', 1, 'File Versioning', '2018-02-24 16:14:25'),
(16, 'file_version_max', 'Maximum file versions to store', '10', 'Maximum number of versions of a file to be stored', 1, 'File Versioning', '2018-02-24 16:14:25'),
(17, 'user_require_approval', 'User Registration Approval', 'true', 'Require approval from system administrator for new user registrations', 1, 'User Registration', '2018-03-02 02:28:30');

-- --------------------------------------------------------

--
-- Table structure for table `backup_target`
--

CREATE TABLE `backup_target` (
  `target_id` int(11) NOT NULL,
  `name` varchar(100) NOT NULL,
  `server` varchar(255) NOT NULL,
  `path` varchar(260) NOT NULL,
  `type` varchar(32) NOT NULL,
  `created` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP,
  `updated` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

--
-- Dumping data for table `backup_target`
--

INSERT INTO `backup_target` (`target_id`, `name`, `server`, `path`, `type`, `created`, `updated`) VALUES
(1, 'Local Storage', 'localhost', '/var/www/html/backup/storage', 'local', '2017-12-26 23:40:48', '2017-12-26 23:40:48'),
(2, 'AWS S3 Storage', '', '', 'AWS S3', '2018-02-18 20:46:39', '2018-02-18 20:46:39');

-- --------------------------------------------------------

--
-- Table structure for table `backup_target_credential`
--

CREATE TABLE `backup_target_credential` (
  `credential_id` int(11) NOT NULL,
  `name` varchar(100) NOT NULL,
  `server` varchar(256) NOT NULL,
  `username` varchar(32) NOT NULL,
  `password` varchar(128) NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

-- --------------------------------------------------------

--
-- Table structure for table `backup_upload`
--

CREATE TABLE `backup_upload` (
  `upload_id` bigint(20) UNSIGNED NOT NULL,
  `file_id` varchar(40) NOT NULL,
  `user_id` int(11) NOT NULL,
  `parts` int(10) UNSIGNED NOT NULL,
  `bytes` bigint(20) UNSIGNED NOT NULL,
  `hash` varchar(40) NOT NULL,
  `compressed` tinyint(1) NOT NULL DEFAULT '0',
  `created` timestamp NULL DEFAULT CURRENT_TIMESTAMP
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

-- --------------------------------------------------------

--
-- Table structure for table `backup_upload_part`
--

CREATE TABLE `backup_upload_part` (
  `upload_part_id` bigint(20) UNSIGNED NOT NULL,
  `upload_id` int(11) NOT NULL,
  `part_number` int(10) UNSIGNED NOT NULL,
  `bytes` bigint(20) UNSIGNED NOT NULL,
  `tmp_file_name` varchar(260) NOT NULL,
  `data` longblob NOT NULL,
  `hash` varchar(40) NOT NULL,
  `compressed` tinyint(1) NOT NULL DEFAULT '0',
  `created` timestamp NULL DEFAULT CURRENT_TIMESTAMP
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

-- --------------------------------------------------------

--
-- Table structure for table `backup_user`
--

CREATE TABLE `backup_user` (
  `active` tinyint(1) NOT NULL DEFAULT '1',
  `user_id` int(11) NOT NULL,
  `user_name` varchar(40) NOT NULL,
  `first_name` varchar(100) NOT NULL,
  `last_name` varchar(100) NOT NULL,
  `email` varchar(255) DEFAULT NULL,
  `address` varchar(256) DEFAULT NULL,
  `city` varchar(100) DEFAULT NULL,
  `state` varchar(100) DEFAULT NULL,
  `zip` varchar(32) DEFAULT NULL,
  `title` varchar(100) DEFAULT NULL,
  `office` varchar(100) DEFAULT NULL,
  `mobile` varchar(32) DEFAULT NULL,
  `access_token` varchar(255) DEFAULT NULL,
  `token_expiry` timestamp NULL DEFAULT NULL,
  `password` varchar(255) NOT NULL,
  `password_set` timestamp NULL DEFAULT NULL,
  `activated` tinyint(1) NOT NULL DEFAULT '0',
  `last_login` timestamp NULL DEFAULT NULL,
  `last_backup` timestamp NULL DEFAULT NULL,
  `source` varchar(32) NOT NULL,
  `created` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP,
  `updated` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

-- --------------------------------------------------------

--
-- Table structure for table `backup_user_activation`
--

CREATE TABLE `backup_user_activation` (
  `activate_id` int(11) NOT NULL,
  `user_id` int(11) NOT NULL,
  `code` varchar(32) NOT NULL,
  `expiry` timestamp NULL DEFAULT NULL,
  `created` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

-- --------------------------------------------------------

--
-- Table structure for table `backup_user_machine`
--

CREATE TABLE `backup_user_machine` (
  `id` int(11) NOT NULL,
  `machine_id` int(11) NOT NULL,
  `user_id` int(11) NOT NULL,
  `target_id` int(11) NOT NULL,
  `last_check_in` timestamp NULL DEFAULT NULL,
  `created` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP,
  `updated` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

-- --------------------------------------------------------

--
-- Table structure for table `backup_user_role`
--

CREATE TABLE `backup_user_role` (
  `user_role_id` int(11) NOT NULL,
  `user_id` int(11) NOT NULL,
  `role_id` int(11) UNSIGNED NOT NULL,
  `added_on` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

-- --------------------------------------------------------

--
-- Table structure for table `backup_user_session`
--

CREATE TABLE `backup_user_session` (
  `session_id` int(11) NOT NULL,
  `user_id` int(11) NOT NULL,
  `session_hash` varchar(128) NOT NULL,
  `ip_address` varchar(32) NOT NULL,
  `last_accessed` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
  `expired` tinyint(1) NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

-- --------------------------------------------------------

--
-- Table structure for table `backup_user_setting`
--

CREATE TABLE `backup_user_setting` (
  `id` int(11) NOT NULL,
  `setting_id` int(11) NOT NULL,
  `user_id` int(11) NOT NULL,
  `value` varchar(255) NOT NULL,
  `created` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

-- --------------------------------------------------------

--
-- Table structure for table `backup_user_target`
--

CREATE TABLE `backup_user_target` (
  `user_target_id` int(11) NOT NULL,
  `user_id` int(11) NOT NULL,
  `target_id` int(11) NOT NULL,
  `created` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

--
-- Indexes for dumped tables
--

--
-- Indexes for table `backup_api_session`
--
ALTER TABLE `backup_api_session`
  ADD PRIMARY KEY (`session_id`),
  ADD KEY `idx_user_id` (`user_id`);

--
-- Indexes for table `backup_client`
--
ALTER TABLE `backup_client`
  ADD PRIMARY KEY (`client_id`),
  ADD KEY `idx_file_name` (`file_name`,`version`);

--
-- Indexes for table `backup_client_setting`
--
ALTER TABLE `backup_client_setting`
  ADD PRIMARY KEY (`setting_id`),
  ADD KEY `idx_name` (`name`);

--
-- Indexes for table `backup_file`
--
ALTER TABLE `backup_file`
  ADD PRIMARY KEY (`file_id`,`user_id`) USING BTREE;

--
-- Indexes for table `backup_ignore_type`
--
ALTER TABLE `backup_ignore_type`
  ADD PRIMARY KEY (`type_id`),
  ADD UNIQUE KEY `idx_file_type` (`file_type`,`type_id`);

--
-- Indexes for table `backup_log`
--
ALTER TABLE `backup_log`
  ADD PRIMARY KEY (`entry_id`,`type`,`user_id`) USING BTREE;

--
-- Indexes for table `backup_machine`
--
ALTER TABLE `backup_machine`
  ADD PRIMARY KEY (`machine_id`),
  ADD UNIQUE KEY `name` (`name`);

--
-- Indexes for table `backup_refresh_token`
--
ALTER TABLE `backup_refresh_token`
  ADD PRIMARY KEY (`user_id`);

--
-- Indexes for table `backup_role`
--
ALTER TABLE `backup_role`
  ADD PRIMARY KEY (`role_id`,`name`) USING BTREE;

--
-- Indexes for table `backup_setting`
--
ALTER TABLE `backup_setting`
  ADD PRIMARY KEY (`setting_id`);

--
-- Indexes for table `backup_target`
--
ALTER TABLE `backup_target`
  ADD PRIMARY KEY (`target_id`);

--
-- Indexes for table `backup_target_credential`
--
ALTER TABLE `backup_target_credential`
  ADD PRIMARY KEY (`credential_id`);

--
-- Indexes for table `backup_upload`
--
ALTER TABLE `backup_upload`
  ADD PRIMARY KEY (`upload_id`,`file_id`,`user_id`) USING BTREE;

--
-- Indexes for table `backup_upload_part`
--
ALTER TABLE `backup_upload_part`
  ADD PRIMARY KEY (`upload_part_id`,`upload_id`) USING BTREE,
  ADD UNIQUE KEY `idx_upload_part` (`upload_id`,`part_number`);

--
-- Indexes for table `backup_user`
--
ALTER TABLE `backup_user`
  ADD PRIMARY KEY (`user_id`),
  ADD UNIQUE KEY `idx_username` (`user_name`) USING BTREE;

--
-- Indexes for table `backup_user_activation`
--
ALTER TABLE `backup_user_activation`
  ADD PRIMARY KEY (`activate_id`),
  ADD UNIQUE KEY `idx_activate_code` (`code`,`user_id`);

--
-- Indexes for table `backup_user_machine`
--
ALTER TABLE `backup_user_machine`
  ADD PRIMARY KEY (`id`),
  ADD UNIQUE KEY `idx_machine_usr` (`machine_id`,`user_id`) USING BTREE;

--
-- Indexes for table `backup_user_role`
--
ALTER TABLE `backup_user_role`
  ADD PRIMARY KEY (`user_role_id`),
  ADD UNIQUE KEY `idx_user_role` (`user_id`,`role_id`) USING BTREE,
  ADD KEY `user_role` (`role_id`);

--
-- Indexes for table `backup_user_session`
--
ALTER TABLE `backup_user_session`
  ADD PRIMARY KEY (`session_id`),
  ADD KEY `hash` (`session_hash`,`session_id`,`user_id`) USING BTREE;

--
-- Indexes for table `backup_user_setting`
--
ALTER TABLE `backup_user_setting`
  ADD PRIMARY KEY (`id`),
  ADD KEY `idx_user_setting` (`setting_id`,`user_id`);

--
-- Indexes for table `backup_user_target`
--
ALTER TABLE `backup_user_target`
  ADD PRIMARY KEY (`user_target_id`),
  ADD KEY `idx_user_target` (`user_id`,`target_id`);

--
-- AUTO_INCREMENT for dumped tables
--

--
-- AUTO_INCREMENT for table `backup_api_session`
--
ALTER TABLE `backup_api_session`
  MODIFY `session_id` int(11) UNSIGNED NOT NULL AUTO_INCREMENT;
--
-- AUTO_INCREMENT for table `backup_client`
--
ALTER TABLE `backup_client`
  MODIFY `client_id` int(11) NOT NULL AUTO_INCREMENT;
--
-- AUTO_INCREMENT for table `backup_client_setting`
--
ALTER TABLE `backup_client_setting`
  MODIFY `setting_id` int(11) NOT NULL AUTO_INCREMENT, AUTO_INCREMENT=8;
--
-- AUTO_INCREMENT for table `backup_ignore_type`
--
ALTER TABLE `backup_ignore_type`
  MODIFY `type_id` int(10) UNSIGNED NOT NULL AUTO_INCREMENT, AUTO_INCREMENT=36;
--
-- AUTO_INCREMENT for table `backup_log`
--
ALTER TABLE `backup_log`
  MODIFY `entry_id` bigint(10) UNSIGNED NOT NULL AUTO_INCREMENT;
--
-- AUTO_INCREMENT for table `backup_machine`
--
ALTER TABLE `backup_machine`
  MODIFY `machine_id` int(11) NOT NULL AUTO_INCREMENT;
--
-- AUTO_INCREMENT for table `backup_role`
--
ALTER TABLE `backup_role`
  MODIFY `role_id` int(11) UNSIGNED NOT NULL AUTO_INCREMENT, AUTO_INCREMENT=3;
--
-- AUTO_INCREMENT for table `backup_setting`
--
ALTER TABLE `backup_setting`
  MODIFY `setting_id` int(11) NOT NULL AUTO_INCREMENT, AUTO_INCREMENT=18;
--
-- AUTO_INCREMENT for table `backup_target`
--
ALTER TABLE `backup_target`
  MODIFY `target_id` int(11) NOT NULL AUTO_INCREMENT, AUTO_INCREMENT=3;
--
-- AUTO_INCREMENT for table `backup_target_credential`
--
ALTER TABLE `backup_target_credential`
  MODIFY `credential_id` int(11) NOT NULL AUTO_INCREMENT;
--
-- AUTO_INCREMENT for table `backup_upload`
--
ALTER TABLE `backup_upload`
  MODIFY `upload_id` bigint(20) UNSIGNED NOT NULL AUTO_INCREMENT;
--
-- AUTO_INCREMENT for table `backup_upload_part`
--
ALTER TABLE `backup_upload_part`
  MODIFY `upload_part_id` bigint(20) UNSIGNED NOT NULL AUTO_INCREMENT;
--
-- AUTO_INCREMENT for table `backup_user`
--
ALTER TABLE `backup_user`
  MODIFY `user_id` int(11) NOT NULL AUTO_INCREMENT;
--
-- AUTO_INCREMENT for table `backup_user_activation`
--
ALTER TABLE `backup_user_activation`
  MODIFY `activate_id` int(11) NOT NULL AUTO_INCREMENT;
--
-- AUTO_INCREMENT for table `backup_user_machine`
--
ALTER TABLE `backup_user_machine`
  MODIFY `id` int(11) NOT NULL AUTO_INCREMENT;
--
-- AUTO_INCREMENT for table `backup_user_role`
--
ALTER TABLE `backup_user_role`
  MODIFY `user_role_id` int(11) NOT NULL AUTO_INCREMENT;
--
-- AUTO_INCREMENT for table `backup_user_session`
--
ALTER TABLE `backup_user_session`
  MODIFY `session_id` int(11) NOT NULL AUTO_INCREMENT;
--
-- AUTO_INCREMENT for table `backup_user_setting`
--
ALTER TABLE `backup_user_setting`
  MODIFY `id` int(11) NOT NULL AUTO_INCREMENT;
--
-- AUTO_INCREMENT for table `backup_user_target`
--
ALTER TABLE `backup_user_target`
  MODIFY `user_target_id` int(11) NOT NULL AUTO_INCREMENT;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
