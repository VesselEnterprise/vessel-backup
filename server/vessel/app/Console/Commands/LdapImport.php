<?php

namespace App\Console\Commands;

use App;
use Illuminate\Console\Command;

class LdapImport extends Command
{
    /**
     * The name and signature of the console command.
     *
     * @var string
     */
    protected $signature = 'ldap:import';

    /**
     * The console command description.
     *
     * @var string
     */
    protected $description = 'LDAP Server Import';

    /**
     * Create a new command instance.
     *
     * @return void
     */
    public function __construct()
    {
        parent::__construct();
    }

    /**
     * Execute the console command.
     *
     * @return mixed
     */
    public function handle()
    {
        $ldapServers = App\LdapServer::all();
				foreach ( $ldapServers as $server ) {
					$server->import();
					echo "Imported from " . $server->name;
				}
    }
}
