<?php

use Illuminate\Support\Facades\Schema;
use Illuminate\Database\Schema\Blueprint;
use Illuminate\Database\Migrations\Migration;

class AlterTableOauthTokens extends Migration
{
    /**
     * Run the migrations.
     *
     * @return void
     */
    public function up()
    {
			/*
      //oauth_access_tokens
      Schema::table('oauth_access_tokens', function (Blueprint $table) {
        $table->dropColumn('user_id');
      });
      Schema::table('oauth_access_tokens', function (Blueprint $table) {
        $table->uuid('user_id')->index()->nullable();
      });

      //oauth_auth_codes
      Schema::table('oauth_auth_codes', function (Blueprint $table) {
        $table->dropColumn('user_id');
      });
      Schema::table('oauth_auth_codes', function (Blueprint $table) {
        $table->uuid('user_id')->index()->nullable();
      });

      //oauth_auth_clients
      Schema::table('oauth_clients', function (Blueprint $table) {
        $table->dropColumn('user_id');
      });
      Schema::table('oauth_clients', function (Blueprint $table) {
        $table->uuid('user_id')->index()->nullable();
      });

      //oauth_auth_codes
      Schema::table('oauth_auth_codes', function (Blueprint $table) {
        $table->dropColumn('user_id');
      });
      Schema::table('oauth_auth_codes', function (Blueprint $table) {
        $table->uuid('user_id')->index()->nullable();
      });
			*/
    }

    /**
     * Reverse the migrations.
     *
     * @return void
     */
    public function down()
    {
        //
    }
}
