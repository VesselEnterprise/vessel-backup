<?php

use App\User;
use App\Role;
use Illuminate\Database\Seeder;

class UserSeeder extends Seeder
{
    /**
     * Run the database seeds.
     *
     * @return void
     */
    public function run()
    {
        //Create the default admin user
				//Add the default admin user
				$admin = new User();
				$admin->first_name = 'Admin';
				$admin->user_name = 'admin';
				$admin->email = 'admin@yourcompany.com';
				$admin->password = Hash::make('vessel'); //Default password
				$admin->source = 'local';
				$admin->save();
				$admin->roles()->attach( Role::where('name', 'admin')->first() );
    }
}
