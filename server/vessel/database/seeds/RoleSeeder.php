<?php

use App\Role;
use Illuminate\Database\Seeder;

class RoleSeeder extends Seeder
{
    /**
     * Run the database seeds.
     *
     * @return void
     */
    public function run()
    {

			DB::table('role')->insert([
				[
					'name' => 'admin',
					'description' => 'Admins have full privileged access'
				],
				[
					'name' => 'user',
					'description' => 'Users can login to manage their files'
				]
			]);

    }
}
