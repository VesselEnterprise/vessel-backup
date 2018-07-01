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
          $table->uuid('id');
          $table->primary('id');
          $table->string('name', 64);
          $table->index('name');
          $table->string('os', 255);
          $table->string('dns_name', 100);
          $table->string('ip_address', 32);
          $table->string('domain', 100);
          $table->string('client_version', 32);
          $table->timestamp('last_check_in');
          $table->timestamp('created_at')->default(DB::raw('CURRENT_TIMESTAMP'));
          $table->timestamp('updated_at')->default(DB::raw('CURRENT_TIMESTAMP on update CURRENT_TIMESTAMP'));
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
