<?php

use Faker\Generator as Faker;
use Spatie\BinaryUuid\HasBinaryUuid;

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

$factory->define(App\File::class, function (Faker $faker) {
    return [
        'file_id' => HasBinaryUuid::encodeUuid( Uuid::generate() ),
        'user_id' => HasBinaryUuid::encodeUuid( Uuid::generate() ),
        'file_name' => 'random_file.jpg',
        'file_path_id' => HasBinaryUuid::encodeUuid( Uuid::generate() ),
        'file_type' => $faker->fileExtension,
        'file_size' => $faker->randomNumber(6, false), // 79907610
        'hash' => sha1( $faker->text(100), true ),
        'uploaded' => $faker->boolean,
        'encrypted' => $faker->boolean,
        'compressed' => $faker->boolean,
        'last_backup' => $faker->dateTimeThisDecade()
    ];
});
