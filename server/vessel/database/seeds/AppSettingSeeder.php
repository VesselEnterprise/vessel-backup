<?php

use Illuminate\Database\Seeder;

class AppSettingSeeder extends Seeder
{
    /**
     * Run the database seeds.
     *
     * @return void
     */
    public function run()
    {
			DB::table('app_setting')->insert([
				[
					'name' => 'max_transfer_speed',
					'value' => '102400',
					'description' => 'Maximum transfer/upload speed (bytes) per second',
					'data_type' => 'int'
				],
				[
					'name' => 'skip_period_dirs',
					'value' => '1',
					'description' => 'Do not read directories that start with a period. (Eg. ".config" ) (0 or 1)',
					'data_type' => 'int'
				],
				[
					'name' => 'new_files_first',
					'value' => '1',
					'description' => 'Always backup recently modified files before older files (0 or 1)',
					'data_type' => 'int'
				]
			]);
    }
}
