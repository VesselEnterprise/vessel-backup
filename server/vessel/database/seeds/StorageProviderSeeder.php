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
						'storage_path' => '/backup',
						'provider_type' => 'aws_s3',
					],
					[
						'provider_id' => HasBinaryUuid::encodeUuid( Uuid::generate() ),
						'provider_name' => 'Azure Blob Storage Example Provider',
						'description' => 'An example storage provider configuration for Azure Blob Storage',
						'server' => 'https://<your-account>.blob.core.windows.net/<your-container>',
						'region' => 'East US',
						'bucket_name' => '<your-container>',
						'storage_path' => '/backup',
						'provider_type' => 'azure_blob',
					],
				];
    }
}
