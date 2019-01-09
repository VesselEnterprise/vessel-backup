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
						$table->string('region')->nullable();
						$table->string('bucket_name')->nullable();
						$table->string('access_id')->nullable();
						$table->string('access_key', 512)->nullable();
						$table->string('storage_path', 255)->nullable();
						$table->string('provider_type', 32);
						$table->integer('priority')->default(0);
						$table->timestamp('created_at')->useCurrent();
						$table->timestamp('updated_at')->default(DB::raw('CURRENT_TIMESTAMP on update CURRENT_TIMESTAMP'));
						$table->index('provider_name');
						$table->index('provider_type');
						$table->boolean('active')->default(true);
            $table->boolean('default')->default(false);
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
