<?php

use Illuminate\Support\Facades\Schema;
use Illuminate\Database\Schema\Blueprint;
use Illuminate\Database\Migrations\Migration;

class CreateDeploymentTable extends Migration
{
    /**
     * Run the migrations.
     *
     * @return void
     */
    public function up()
    {
        Schema::create('deployment', function (Blueprint $table) {
            $table->increments('id');
						$table->string('deployment_name');
						$table->string('deployment_key', 32 );
						$table->timestamp('expires_at')->nullable();
						$table->boolean('never_expires');
						$table->timestamp('created_at')->useCurrent();
	          $table->timestamp('updated_at')->default(DB::raw('CURRENT_TIMESTAMP on update CURRENT_TIMESTAMP'));
            //$table->timestamps();
						$table->index('deployment_key');
        });
    }

    /**
     * Reverse the migrations.
     *
     * @return void
     */
    public function down()
    {
        Schema::dropIfExists('deployment');
    }
}
