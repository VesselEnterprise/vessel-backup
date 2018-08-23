<?php

use Illuminate\Support\Facades\Schema;
use Illuminate\Database\Schema\Blueprint;
use Illuminate\Database\Migrations\Migration;

class CreateAppclientTable extends Migration
{
    /**
     * Run the migrations.
     *
     * @return void
     */
    public function up()
    {
        Schema::create('app_client', function (Blueprint $table) {
          $table->uuid('client_id');
          $table->primary('client_id');
          $table->string('client_name', 64)->unique();
          $table->string('os', 255)->nullable();
          $table->string('dns_name', 100)->nullable();
          $table->string('ip_address', 32)->nullable();
          $table->string('domain', 100)->nullable();
          $table->string('client_version', 32)->nullable();
					$table->string('token', 32)->unique();
          $table->timestamp('last_check_in')->nullable();
					$table->timestamp('last_backup')->nullable();
          $table->timestamp('created_at')->default(DB::raw('CURRENT_TIMESTAMP'));
          $table->timestamp('updated_at')->default(DB::raw('CURRENT_TIMESTAMP on update CURRENT_TIMESTAMP'));
					$table->index('token');
        });
    }

    /**
     * Reverse the migrations.
     *
     * @return void
     */
    public function down()
    {
        Schema::dropIfExists('app_client');
    }
}
