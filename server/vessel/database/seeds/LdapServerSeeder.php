<?php

use Illuminate\Database\Seeder;

class LdapServerSeeder extends Seeder
{
    /**
     * Run the database seeds.
     *
     * @return void
     */
    public function run()
    {
        $servers = [
					[
						'name' => 'Test LDAP Server', //https://www.forumsys.com/tutorials/integration-how-to/ldap/online-ldap-test-server/
						'server' => 'ldap.forumsys.com',
						'username' => 'cn=read-only-admin,dc=example,dc=com',
						'port' => 389,
						'dn' => 'dc=example,dc=com',
						'password' => 'password',
						'type' => 'open_ldap'
					]
				];

				DB::table('ldap_server')->insert($servers);

    }
}
