<?php

use App\AppClient;
use Illuminate\Database\Seeder;

class TestAppClientSeeder extends Seeder
{
    /**
     * Run the database seeds.
     *
     * @return void
     */
    public function run()
    {
        $clients = factory(AppClient::class,10000)->create();
    }
}
