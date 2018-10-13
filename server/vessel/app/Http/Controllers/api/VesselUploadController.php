<?php

namespace App\Http\Controllers\api;

use App;
use Illuminate\Http\Request;
use App\Http\Controllers\Controller;
use Storage;
use Carbon\Carbon;

class VesselUploadController extends Controller
{

		/**
		* Upload a file
		*
		* @return \Illuminate\Http\Response
		*/
		public function uploadFile(Request $request, $filePath) {

			//Get Storage Provider
			$providerId = $request->header('X-Vessel-Storage-Provider');
			$storageProvider = App\StorageProvider::withUuid($providerId)->firstOrFail();

			//Get User
			$userId = $request->header('X-Vessel-User-Id');
			$user = App\User::withUuid($userId)->firstOrFail();

			//File Content
			$fileContents = $request->getContent();

			//Handle file path information
			$pathParts = pathinfo($filePath);
			$fpHashRaw = sha1( $pathParts['dirname'], true );
			$dbFilePath = App\FilePath::where(['hash' => $fpHashRaw, 'user_id' => $user->user_id])->first();
			$fileHash = sha1( $fileContents, true );

			if ( !$dbFilePath ) {

				//Duplicate check should only occur if this is a new file_path
				$preventDuplicate = App\Setting::where(['setting_name' => 'prevent_duplicate_hash', 'value' => 'true'])->first();
				if ( $preventDuplicate ) {
					$duplicate = App\File::where(['hash' => $fileHash, 'user_id' => $user->user_id, 'provider_id' => $storageProvider->provider_id])->first();
					if ( $duplicate ) {
						return response()->json(['error' => 'Duplicate file: File has already been uploaded'], 400);
					}
				}

				$dbFilePath = new App\FilePath;
				$dbFilePath->user_id = $user->user_id;
				$dbFilePath->file_path = $filePath;
				$dbFilePath->hash = $fpHashRaw;
				$dbFilePath->save();

			}

			//Write to storage
			Storage::disk('vessel')->put($filePath, $fileContents);

			$file = new App\File;
			$file->user_id = $user->user_id;
			$file->file_name = $pathParts['filename'];
			$file->file_path_id = $dbFilePath->path_id;
			$file->provider_id = $storageProvider->provider_id;
			$file->file_type = $pathParts['extension'];
			$file->file_size = strlen($fileContents);
			$file->hash = $fileHash;
			$file->uploaded = 1;
			$file->last_backup = Carbon::now();
			$file->save();

			return response()->json(['result' => $file]);

		}

		/**
		* Initialize a multipart upload
		*
		* @return \Illuminate\Http\Response
		*/
		public function initMultiPartUpload(Request $request) {


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
