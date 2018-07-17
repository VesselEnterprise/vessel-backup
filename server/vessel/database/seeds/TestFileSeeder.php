<?php

use App\File;
use Illuminate\Database\Seeder;

class TestFileSeeder extends Seeder
{
    /**
     * Run the database seeds.
     *
     * @return void
     */
    public function run()
    {
        //Create 50,000 files
        for ( $i=0; $i < 1; $i++) {
          $files = factory(File::class,10000)->create();
        }
    }
}
