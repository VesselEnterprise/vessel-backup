<?php

use Illuminate\Support\Facades\Schema;
use Illuminate\Database\Schema\Blueprint;
use Illuminate\Database\Migrations\Migration;

class CreateSettingTable extends Migration
{
    /**
     * Run the migrations.
     *
     * @return void
     */
    public function up()
    {
        Schema::create('setting', function (Blueprint $table) {
            $table->increments('id');
            $table->string('name', 32)->unique();
            $table->string('display_name', 100);
            $table->string('value', 255);
            $table->text('description');
            $table->boolean('active')->default(1);
            $table->string('type', 32);
            $table->timestamp('created_at')->default(DB::raw('CURRENT_TIMESTAMP'));
            $table->timestamp('updated_at')->default(DB::raw('CURRENT_TIMESTAMP on update CURRENT_TIMESTAMP'));
            //$table->timestamps();
        });

        $initData = array(
          array(
            'name' => 'temp_storage_path',
            'display_name' => 'Temporary Storage Path',
            'value' => '/var/www/vessel/storage/parts',
            'description' => 'Filesystem path where temporary file upload data should be stored',
            'type' => 'Upload'
          ),
          array(
            'name' => 'ldap_server',
            'display_name' => 'LDAP Server',
            'value' => '',
            'description' => 'LDAP server used for importing users and computers',
            'type' => 'LDAP'
          ),
          array(
            'name' => 'ldap_user',
            'display_name' => 'LDAP Username',
            'value' => 'vessel',
            'description' => 'LDAP username used for connecting to LDAP server',
            'type' => 'LDAP'
          ),
          array(
            'name' => 'ldap_port',
            'display_name' => 'LDAP Port',
            'value' => '389',
            'description' => 'Port used to connect to the LDAP server',
            'type' => 'LDAP'
          ),
          array(
            'name' => 'ldap_pwd',
            'display_name' => 'LDAP Password',
            'value' => 'vessel',
            'description' => 'LDAP user password used to connect to LDAP server',
            'type' => 'LDAP'
          ),
          array(
            'name' => 'ldap_user_tree',
            'display_name' => 'LDAP User DN',
            'value' => 'OU=Users,DC=some,DC=org',
            'description' => 'Base DN of where to search for LDAP user objects',
            'type' => 'LDAP'
          ),
          array(
            'name' => 'ldap_pc_tree',
            'display_name' => 'LDAP Computer DN',
            'value' => 'OU=Computers,DC=some,DC=org',
            'description' => 'Base DN of where to search for LDAP computers',
            'type' => 'LDAP'
          ),
          array(
            'name' => 'ldap_activate_code',
            'display_name' => 'LDAP User Activation Code',
            'value' => 'default',
            'description' => 'Users who are imported via LDAP will all use the same activation code. When the client checks into the server for the first time, they will be able to activate using the default activation code. It is recommended to NOT change this value',
            'type' => 'LDAP'
          ),
          array(
            'name' => 'file_version_size',
            'display_name' => 'Maximum file size for file versioning',
            'value' => '5242880',
            'description' => 'Maximum file size used for file versioning. Files greater than this size will not be versioned',
            'type' => 'File Versioning'
          ),
          array(
            'name' => 'file_version_max',
            'display_name' => 'Maximum file versions to store',
            'value' => '5',
            'description' => 'Maximum number of versions of a file to be stored',
            'type' => 'File Versioning'
          ),
          array(
            'name' => 'max_upload_size',
            'display_name' => 'Max Upload Filesize',
            'value' => '2147483648',
            'description' => 'Maximum file size for an upload. Default = 2GB',
            'type' => 'File Upload'
          )
        );

        DB::table('setting')->insert($initData);
    }

    /**
     * Reverse the migrations.
     *
     * @return void
     */
    public function down()
    {
        Schema::dropIfExists('setting');
    }
}
