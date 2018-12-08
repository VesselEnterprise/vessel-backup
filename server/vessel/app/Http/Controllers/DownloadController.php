<?php

namespace App\Http\Controllers;

use App;
use Illuminate\Http\Request;
use Illuminate\Support\Facades\Auth;

//AWS
use Aws\Credentials;
use Aws\S3\S3Client;
use Aws\S3\Exception\S3Exception;

class DownloadController extends Controller
{

		public function downloadFile(Request $request, $id)
		{

			//Get File and Determine the Storage Provider
			$file = App\File::withUuid($id)->firstOrFail();

			if ( $file->storageProvider->provider_type == "aws_s3") {
				return $this->downloadS3File($file);
			}

		}

		private function downloadS3File($file) {

			try
			{

				// Instantiate an Amazon S3 client.
				$client = new S3Client([
					'version' => 'latest',
					'region'  => $file->storageProvider->region,
					'credentials' => [
						'key' => $file->storageProvider->access_id,
						'secret' => $file->storageProvider->access_key
					],
				]);
				$client->registerStreamWrapper();

				//Build file path
				$strFilePath = ltrim($file->filePath->file_path, '/');

				// Get the object.
				/*
				$result = $client->getObject([
					'Bucket' => $file->storageProvider->bucket_name,
					'Key'    => $file->storageProvider->storage_path . "/" . $file->user->user_id_text . "/" . $strFilePath . '/' . $file->file_name
				]);
				*/

				$key = $file->storageProvider->storage_path . "/" . $file->user->user_id_text . "/" . $strFilePath . '/' . $file->file_name;

				//Create the file stream
				$stream = @fopen('s3://' . $file->storageProvider->bucket_name . '/' . $key, 'r');

				if ( !$stream ) {
					abort(404, "The file does not exist or the stream could not be created");
				}

				//Stream the download
				return response()->streamDownload(function() use ($file, $stream) {

					//Stream the download
					while (!feof($stream))
					{
						echo fread($stream, 1024);
					}
					fclose($stream);

				}, $file->file_name);


			}
			catch (S3Exception $e)
			{
				echo $e->getMessage() . PHP_EOL;
			}

		}

    /**
     * Display a listing of the resource.
     *
     * @return \Illuminate\Http\Response
     */
    public function index()
    {
        //
    }

    /**
     * Show the form for creating a new resource.
     *
     * @return \Illuminate\Http\Response
     */
    public function create()
    {
        //
    }

    /**
     * Store a newly created resource in storage.
     *
     * @param  \Illuminate\Http\Request  $request
     * @return \Illuminate\Http\Response
     */
    public function store(Request $request)
    {
        //
    }

    /**
     * Display the specified resource.
     *
     * @param  int  $id
     * @return \Illuminate\Http\Response
     */
    public function show($id)
    {
        //
    }

    /**
     * Show the form for editing the specified resource.
     *
     * @param  int  $id
     * @return \Illuminate\Http\Response
     */
    public function edit($id)
    {
        //
    }

    /**
     * Update the specified resource in storage.
     *
     * @param  \Illuminate\Http\Request  $request
     * @param  int  $id
     * @return \Illuminate\Http\Response
     */
    public function update(Request $request, $id)
    {
        //
    }

    /**
     * Remove the specified resource from storage.
     *
     * @param  int  $id
     * @return \Illuminate\Http\Response
     */
    public function destroy($id)
    {
        //
    }
}
