<?php

use Illuminate\Database\Seeder;
use Spatie\BinaryUuid\HasBinaryUuid;

class StorageProviderSeeder extends Seeder
{
    /**
     * Run the database seeds.
     *
     * @return void
     */
    public function run()
    {
        $providers = [
					[
						'provider_id' => HasBinaryUuid::encodeUuid( Uuid::generate() ),
						'provider_name' => 'AWS S3 Example Provider',
						'description' => 'An example storage provider configuration for AWS S3',
						'server' => 'https://<your-bucket>.s3-us-east-2.amazonaws.com',
						'region' => 'us-east-2',
						'bucket_name' => '<your-bucket>',
						'access_id' => '<your-access-id>',
						'storage_path' => '/backup',
						'provider_type' => 'aws_s3',
						'active' => false,
            'default' => true
					],
					[
						'provider_id' => HasBinaryUuid::encodeUuid( Uuid::generate() ),
						'provider_name' => 'Azure Blob Storage Example Provider',
						'description' => 'An example storage provider configuration for Azure Blob Storage',
						'server' => 'https://<your-account>.blob.core.windows.net',
						'region' => 'East US',
						'bucket_name' => '<your-container>',
						'access_id' => '<your-account>',
						'storage_path' => '/backup',
						'provider_type' => 'azure_blob',
						'active' => false,
            'default' => false
					],
				];

				DB::table('storage_provider')->insert($providers);
    }
}
