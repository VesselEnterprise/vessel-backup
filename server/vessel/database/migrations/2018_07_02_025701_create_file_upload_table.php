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
						$table->uuid('file_id');
						$table->uuid('user_id');
						$table->unsignedInteger('parts')->default(0);
						$table->unsignedBigInteger('total_bytes')->default(0);
						$table->boolean('compressed')->default(0);
						//$table->blob('hash', 20);
						$table->timestamp('created_at')->default(DB::raw('CURRENT_TIMESTAMP'));
            $table->timestamp('updated_at')->default(DB::raw('CURRENT_TIMESTAMP on update CURRENT_TIMESTAMP'));
            //$table->timestamps();
						$table->index('file_id');
						$table->index('user_id');
        });

				DB::statement("ALTER TABLE `file_upload` ADD `hash` BINARY(20) NOT NULL AFTER `total_bytes`");
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
