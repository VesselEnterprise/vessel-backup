<?php

use Illuminate\Database\Seeder;

class FileIgnoreTypeSeeder extends Seeder
{
    /**
     * Run the database seeds.
     *
     * @return void
     */
    public function run()
    {
			DB::table('file_ignore_type')->insert([
				['file_type' => '.7z'],
				['file_type' => '.avi'],
				['file_type' => '.bat'],
				['file_type' => '.cab'],
				['file_type' => '.cmd'],
				['file_type' => '.dll'],
				['file_type' => '.exe'],
				['file_type' => '.flac'],
				['file_type' => '.inf'],
				['file_type' => '.ini'],
				['file_type' => '.iso'],
				['file_type' => '.itdb'],
				['file_type' => '.itl'],
				['file_type' => '.m3u'],
				['file_type' => '.m4a'],
				['file_type' => '.m4p'],
				['file_type' => '.m4v'],
				['file_type' => '.mkv'],
				['file_type' => '.mov'],
				['file_type' => '.mp2'],
				['file_type' => '.mp3'],
				['file_type' => '.mp4'],
				['file_type' => '.mpeg'],
				['file_type' => '.mpg'],
				['file_type' => '.msi'],
				['file_type' => '.nds'],
				['file_type' => '.ost'],
				['file_type' => '.pbf'],
				['file_type' => '.tmp'],
				['file_type' => '.vmdk'],
				['file_type' => '.vob'],
				['file_type' => '.wav'],
				['file_type' => '.wma'],
				['file_type' => '.wmv']
			]);
    }
}
