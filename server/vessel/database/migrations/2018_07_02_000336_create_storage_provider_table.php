<?php

use Illuminate\Support\Facades\Schema;
use Illuminate\Database\Schema\Blueprint;
use Illuminate\Database\Migrations\Migration;

class CreateStorageProviderTable extends Migration
{
    /**
     * Run the migrations.
     *
     * @return void
     */
    public function up()
    {
        Schema::create('storage_provider', function (Blueprint $table) {
            $table->uuid('provider_id');
						$table->primary('provider_id');
						$table->string('provider_name', 36)->unique();
						$table->text('description')->nullable();
						$table->string('server');
						$table->string('region');
						$table->string('bucket_name');
						$table->string('access_id');
						$table->string('access_key', 512);
						$table->string('storage_path', 255);
						$table->string('provider_type', 32);
						$table->integer('priority');
						$table->timestamp('created_at')->useCurrent();
						$table->timestamp('updated_at')->default(DB::raw('CURRENT_TIMESTAMP on update CURRENT_TIMESTAMP'));
						$table->index('provider_name');
						$table->index('provider_type');
            //$table->timestamps();
        });
    }

    /**
     * Reverse the migrations.
     *
     * @return void
     */
    public function down()
    {
        Schema::dropIfExists('storage_provider');
    }
}
