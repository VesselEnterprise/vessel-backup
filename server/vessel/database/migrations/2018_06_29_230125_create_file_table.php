<?php

use Illuminate\Support\Facades\Schema;
use Illuminate\Database\Schema\Blueprint;
use Illuminate\Database\Migrations\Migration;

class CreateFileTable extends Migration
{
    /**
     * Run the migrations.
     *
     * @return void
     */
    public function up()
    {
        Schema::create('file', function (Blueprint $table) {

          //Create columns
          $table->uuid('file_id')->primary();
          $table->uuid('user_id')->index();
          $table->string('file_name', 255);
          $table->uuid('file_path_id')->index();
					$table->uuid('provider_id')->index();
          $table->string('file_type', 16 )->index()->nullable();
          $table->unsignedBigInteger('file_size');
          //$table->binary('hash')->comment('SHA-1 hash of the file contents'); //Laravel incorrectly creates a BLOB here for MySQL
          $table->boolean('uploaded')->comment('Indicates whether or not the upload has been completed');
          $table->boolean('encrypted')->comment('Indicates whether or not the upload has been encrypted');
          $table->boolean('compressed')->comment('Indicates whether or not the upload has been compressed');
          $table->timestamp('last_backup')->nullable();
          $table->timestamp('created_at')->useCurrent();
          $table->timestamp('updated_at')->default(DB::raw('CURRENT_TIMESTAMP on update CURRENT_TIMESTAMP'));

        });

        DB::statement("ALTER TABLE `file` ADD `hash` BINARY(20) NULL AFTER `file_size`");
    }

    /**
     * Reverse the migrations.
     *
     * @return void
     */
    public function down()
    {
        Schema::dropIfExists('file');
    }
}
