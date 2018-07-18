<?php

use Illuminate\Database\Seeder;

class SettingSeeder extends Seeder
{
    /**
     * Run the database seeds.
     *
     * @return void
     */
    public function run()
    {
			$initData = array(
				array(
					'setting_name' => 'temp_storage_path',
					'display_name' => 'Temporary Storage Path',
					'value' => '/var/www/vessel/storage/parts',
					'description' => 'Filesystem path where temporary file upload data should be stored',
					'category' => 'Upload',
					'data_type' => 'string'
				),
				array(
					'setting_name' => 'ldap_server',
					'display_name' => 'LDAP Server',
					'value' => '',
					'description' => 'LDAP server used for importing users and computers',
					'category' => 'LDAP',
					'data_type' => 'string'
				),
				array(
					'setting_name' => 'ldap_user',
					'display_name' => 'LDAP Username',
					'value' => 'vessel',
					'description' => 'LDAP username used for connecting to LDAP server',
					'category' => 'LDAP',
					'data_type' => 'string'
				),
				array(
					'setting_name' => 'ldap_port',
					'display_name' => 'LDAP Port',
					'value' => '389',
					'description' => 'Port used to connect to the LDAP server',
					'category' => 'LDAP',
					'data_type' => 'integer'
				),
				array(
					'setting_name' => 'ldap_pwd',
					'display_name' => 'LDAP Password',
					'value' => 'vessel',
					'description' => 'LDAP user password used to connect to LDAP server',
					'category' => 'LDAP',
					'data_type' => 'string'
				),
				array(
					'setting_name' => 'ldap_user_tree',
					'display_name' => 'LDAP User DN',
					'value' => 'OU=Users,DC=some,DC=org',
					'description' => 'Base DN of where to search for LDAP user objects',
					'category' => 'LDAP',
					'data_type' => 'string'
				),
				array(
					'setting_name' => 'ldap_pc_tree',
					'display_name' => 'LDAP Computer DN',
					'value' => 'OU=Computers,DC=some,DC=org',
					'description' => 'Base DN of where to search for LDAP computers',
					'category' => 'LDAP',
					'data_type' => 'string'
				),
				array(
					'setting_name' => 'file_version_size',
					'display_name' => 'Maximum file size for file versioning',
					'value' => '5242880',
					'description' => 'Maximum file size used for file versioning. Files greater than this size will not be versioned (bytes)',
					'category' => 'File Versioning',
					'data_type' => 'integer'
				),
				array(
					'setting_name' => 'file_version_max',
					'display_name' => 'Maximum file versions to store',
					'value' => '5',
					'description' => 'Maximum number of versions of a file to be stored',
					'category' => 'File Versioning',
					'data_type' => 'integer'
				),
				array(
					'setting_name' => 'max_upload_size',
					'display_name' => 'Max Upload Filesize',
					'value' => '2147483648',
					'description' => 'Maximum file size for an upload. Default = 2GB',
					'category' => 'File Upload',
					'data_type' => 'integer'
				)
			);

			DB::table('setting')->insert($initData);
    }
}
