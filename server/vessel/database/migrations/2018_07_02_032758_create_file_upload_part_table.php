<?php

use Illuminate\Support\Facades\Schema;
use Illuminate\Database\Schema\Blueprint;
use Illuminate\Database\Migrations\Migration;

class CreateFileUploadPartTable extends Migration
{
    /**
     * Run the migrations.
     *
     * @return void
     */
    public function up()
    {
        Schema::create('file_upload_part', function (Blueprint $table) {
            $table->uuid('part_id');
						$table->primary('part_id');
						$table->uuid('upload_id');
						$table->unsignedInteger('part_number')->default(0);
						$table->unsignedBigInteger('total_bytes')->default(0);
						$table->boolean('compressed')->default(0);
						$table->timestamp('created_at')->default(DB::raw('CURRENT_TIMESTAMP'));
            $table->timestamp('updated_at')->default(DB::raw('CURRENT_TIMESTAMP on update CURRENT_TIMESTAMP'));
            //$table->timestamps();
						$table->index('upload_id');
        });

				DB::statement("ALTER TABLE `file_upload_part` ADD `hash` BINARY(20) NOT NULL AFTER `total_bytes`");
    }

    /**
     * Reverse the migrations.
     *
     * @return void
     */
    public function down()
    {
        Schema::dropIfExists('file_upload_part');
    }
}
