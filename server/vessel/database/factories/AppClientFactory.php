<?php

use Faker\Generator as Faker;

/*
|--------------------------------------------------------------------------
| Model Factories
|--------------------------------------------------------------------------
|
| This directory should contain each of the model factory definitions for
| your application. Factories provide a convenient way to generate new
| model instances for testing / seeding your application's database.
|
*/

$factory->define(App\AppClient::class, function (Faker $faker) {
    return [
        'client_name' => utf8_encode($faker->domainWord),
        'os' => utf8_encode($faker->userAgent),
        'dns_name' => utf8_encode($faker->domainWord),
        'ip_address' => $faker->ipv4,
        'domain' => utf8_encode($faker->domainName),
        'client_version' => '1.0',
        'last_check_in' => $faker->dateTimeThisDecade(),
				'last_backup' => $faker->dateTimeThisDecade(),
				'token' => App\AppClient::generateToken()
    ];
});
