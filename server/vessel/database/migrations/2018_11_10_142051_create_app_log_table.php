<?php

use Illuminate\Support\Facades\Schema;
use Illuminate\Database\Schema\Blueprint;
use Illuminate\Database\Migrations\Migration;

class CreateAppLogTable extends Migration
{
    /**
     * Run the migrations.
     *
     * @return void
     */
    public function up()
    {
        Schema::create('app_log', function (Blueprint $table) {
            $table->increments('id');
						$table->uuid('user_id')->index();
						$table->uuid('client_id')->index();
						$table->text('message')->nullable();
						$table->text('payload')->nullable();
						$table->string('type', 60)->nullable();
						$table->string('exception', 60)->nullable();
						$table->integer('code')->nullable();
						$table->boolean('error')->default(0)->index();
						$table->timestamp('logged_at');
						$table->timestamp('created_at')->useCurrent();
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
        Schema::dropIfExists('app_log');
    }
}
