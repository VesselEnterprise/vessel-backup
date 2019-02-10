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
				),
				array(
					'setting_name' => 'enable_download_portal',
					'display_name' => 'Enable Download Portal',
					'value' => 'true',
					'description' => 'Enable or disable the user download portal',
					'category' => 'File Management',
					'data_type' => 'boolean'
				),
				array(
					'setting_name' => 'prevent_duplicate_hash',
					'display_name' => 'Prevent Duplicate File Hashes',
					'value' => 'true',
					'description' => 'Prevent files with the same SHA-1 hashes from being uploaded again for the same user',
					'category' => 'File Management',
					'data_type' => 'boolean'
				)
			);

			DB::table('setting')->insert($initData);
    }
}
