<?php

use Illuminate\Support\Facades\Schema;
use Illuminate\Database\Schema\Blueprint;
use Illuminate\Database\Migrations\Migration;

class CreateFileUploadTable extends Migration
{
    /**
     * Run the migrations.
     *
     * @return void
     */
    public function up()
    {
        Schema::create('file_upload', function (Blueprint $table) {
            $table->uuid('upload_id');
						$table->primary('upload_id');
						$table->uuid('file_id')->index();
						$table->uuid('user_id')->index();
						$table->uuid('client_id')->index();
						$table->unsignedInteger('parts')->default(0);
						$table->unsignedBigInteger('total_bytes')->default(0);
						$table->boolean('uploaded')->default(0);
						$table->boolean('compressed')->default(0);
						$table->timestamp('created_at')->useCurrent();
            $table->timestamp('updated_at')->default(DB::raw('CURRENT_TIMESTAMP on update CURRENT_TIMESTAMP'));
            //$table->timestamps();
        });

				DB::statement("ALTER TABLE `file_upload` ADD `hash` BINARY(20) NOT NULL AFTER `total_bytes`"); //$table->blob('hash', 20);
    }

    /**
     * Reverse the migrations.
     *
     * @return void
     */
    public function down()
    {
        Schema::dropIfExists('file_upload');
    }
}
