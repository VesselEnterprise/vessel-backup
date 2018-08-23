<?php

use Illuminate\Support\Facades\Schema;
use Illuminate\Database\Schema\Blueprint;
use Illuminate\Database\Migrations\Migration;

class CreateFilePathTable extends Migration
{
    /**
     * Run the migrations.
     *
     * @return void
     */
    public function up()
    {
        Schema::create('file_path', function (Blueprint $table) {
					$table->uuid('path_id');
					$table->primary('path_id');
					$table->uuid('user_id');
					$table->string('file_path', 255);
					$table->string('hash');
					$table->timestamp('created_at')->useCurrent();
					$table->timestamp('updated_at')->default(DB::raw('CURRENT_TIMESTAMP on update CURRENT_TIMESTAMP'));
					$table->index(['user_id','hash']);
        });

				DB::statement("ALTER TABLE `file_path` CHANGE `hash` `hash` BINARY(20) NOT NULL");
    }

    /**
     * Reverse the migrations.
     *
     * @return void
     */
    public function down()
    {
        Schema::dropIfExists('file_path');
    }
}
