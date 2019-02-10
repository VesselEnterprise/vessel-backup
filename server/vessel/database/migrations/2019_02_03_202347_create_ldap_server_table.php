<?php

use Illuminate\Support\Facades\Schema;
use Illuminate\Database\Schema\Blueprint;
use Illuminate\Database\Migrations\Migration;

class CreateLdapServerTable extends Migration
{
    /**
     * Run the migrations.
     *
     * @return void
     */
    public function up()
    {
        Schema::create('ldap_server', function (Blueprint $table) {
            $table->increments('id');
						$table->string('name');
						$table->string('server');
						$table->string('username',255);
						$table->string('password', 255);
						$table->string('dn', 255);
						$table->integer('port')->default(389);
						$table->string('type')->default('active_directory');
						$table->boolean('active')->default(1);
						$table->timestamp('created_at')->useCurrent();
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
        Schema::dropIfExists('ldap_server');
    }
}
