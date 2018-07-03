<?php

namespace Tests\Unit;

use Tests\TestCase;
use Illuminate\Foundation\Testing\WithFaker;
use Illuminate\Foundation\Testing\RefreshDatabase;
use App\User;
use App\File;
use App\FileUpload;
use App\FileUploadPart;
use App\StorageProvider;
use App\StorageProviderFile;
use Illuminate\Support\Facades\Hash;

class FileRelationshipTest extends TestCase
{
    /**
     * A basic test example.
     *
     * @return void
     */
    public function testRelationships()
    {

				//Create a new user to test with
				$user = User::firstOrNew(['email' => 'test@yourcompany.com']);
				$user->first_name = 'Test';
				$user->last_name = 'User';
				$user->password = Hash::make('test');
				$user->save();

				//Create a new file and associate the file with the user
				$file = new File();
				$file->file_name = 'test.jpg';
				$file->file_type = '.jpg';
				$file->file_size = 1000000;
				$file->uploaded = 1;

				$user->files()->save($file);

				//Verify the user_id's match
				$this->assertTrue($file->user_id == $user->user_id);

				//Create a file upload and associate it with the file
				$upload = new FileUpload();
				$upload->user_id = $user->user_id;
				$upload->parts = 100;
				$upload->total_bytes = 1000000;

				$file->uploads()->save($upload);

				//Verify file_id is populated
				$this->assertTrue($upload->file_id == $file->file_id);

				//Verify user_id is populated for upload
				$this->assertTrue($upload->user_id == $user->user_id);

				//Add some parts to the file upload
				$upload->parts()->saveMany([
					new FileUploadPart(['part_number' => 1, 'total_bytes' => 1000000]),
					new FileUploadPart(['part_number' => 2, 'total_bytes' => 1000000]),
					new FileUploadPart(['part_number' => 3, 'total_bytes' => 1000000]),
					new FileUploadPart(['part_number' => 4, 'total_bytes' => 1000000])
				]);

				//Verify there are 4 upload parts
				$this->assertTrue( $upload->parts()->count() == 4 );

				//Create a new storage provider
				$storageProvider = StorageProvider::firstOrNew(['provider_name' => 'Local Filesystem']);
				$storageProvider->server = 'https://localhost';
				$storageProvider->storage_path = '/var/www/html/upload';
				$storageProvider->provider_type = 'local';
				$storageProvider->save();

				//Associate the file w/ a storage provider
				$providerFile = new StorageProviderFile();
				$providerFile->file_id = $file->file_id;

				$storageProvider->files()->save($providerFile);

				//Validate the number of storage provider files w/ the same user id
				$this->assertTrue( $storageProvider->files()->count() >= 1 );

        $this->assertTrue(true);
    }
}
