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
            $table->uuid('user_id');
            $table->primary('user_id');
            $table->string('email')->unique();
            $table->string('user_name', 40)->unique();
            $table->string('password', 255);
            $table->boolean('active')->default(true);
            $table->string('first_name', 100);
            $table->string('last_name', 100);
            $table->text('address');
            $table->string('city', 100);
            $table->string('state', 100);
            $table->string('zip', 32);
            $table->string('title', 100);
            $table->string('office', 100);
            $table->string('mobile', 32);
            $table->timestamp('last_login')->nullable();
            $table->string('source', 32);

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
