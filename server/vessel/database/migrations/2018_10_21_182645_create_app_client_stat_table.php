<?php

use Illuminate\Support\Facades\Schema;
use Illuminate\Database\Schema\Blueprint;
use Illuminate\Database\Migrations\Migration;

class CreateAppClientStatTable extends Migration
{
    /**
     * Run the migrations.
     *
     * @return void
     */
    public function up()
    {
        Schema::create('app_client_stat', function (Blueprint $table) {
            $table->increments('id');
						$table->uuid('client_id');
						$table->uuid('user_id');
						$table->string('stat_name', 32);
						$table->integer('value');
						$table->timestamp('created_at')->default(DB::raw('CURRENT_TIMESTAMP'));
	          $table->timestamp('updated_at')->default(DB::raw('CURRENT_TIMESTAMP on update CURRENT_TIMESTAMP'));
            //$table->timestamps();
						$table->index(['client_id', 'stat_name']);
        });
    }

    /**
     * Reverse the migrations.
     *
     * @return void
     */
    public function down()
    {
        Schema::dropIfExists('app_client_stat');
    }
}
