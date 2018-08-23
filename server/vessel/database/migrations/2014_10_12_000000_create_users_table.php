<?php

use Illuminate\Support\Facades\Schema;
use Illuminate\Database\Schema\Blueprint;
use Illuminate\Database\Migrations\Migration;
use Spatie\BinaryUuid\HasBinaryUuid;
use App\User;
use App\Role;

class CreateUsersTable extends Migration
{
    /**
     * Run the migrations.
     *
     * @return void
     */
    public function up()
    {
        Schema::create('users', function (Blueprint $table) {
						$table->increments('id');
            $table->uuid('user_id')->unique();
            $table->string('email')->unique();
            $table->string('user_name', 40)->unique();
            $table->string('password', 255)->nullable();
						$table->string('api_token', 1200)->nullable();
            $table->boolean('active')->default(true);
            $table->string('first_name', 100)->nullable();
            $table->string('last_name', 100)->nullable();
            $table->text('address')->nullable();
            $table->string('city', 100)->nullable();
            $table->string('state', 100)->nullable();
            $table->string('zip', 32)->nullable();
            $table->string('title', 100)->nullable();
            $table->string('office', 100)->nullable();
            $table->string('mobile', 32)->nullable();
            $table->timestamp('last_login')->nullable();
						$table->timestamp('last_backup')->nullable();
						$table->timestamp('last_check_in')->nullable();
            $table->string('source', 32)->nullable();

            $table->rememberToken();

            $table->timestamp('created_at')->default(DB::raw('CURRENT_TIMESTAMP'));
            $table->timestamp('updated_at')->default(DB::raw('CURRENT_TIMESTAMP on update CURRENT_TIMESTAMP'));
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
        Schema::dropIfExists('users');
    }
}
