<?php

namespace App\Http\Controllers\api;

use App;
use Illuminate\Http\Request;
use App\Http\Controllers\Controller;

class UploadController extends Controller
{

		public function initUpload(Request $request)
		{

			$client = App\AppClient::where('token', $request->bearerToken() )->first();

			if ( !$client ) {
				return response()->json(['error' => 'Bad client'], 400);
			}

			//Find the Associated User Id
			$user = App\User::where(['user_name' => $request->input('user_name')])->first();

			//User must be in the database prior to init
			if ( !$user ) {
				return response()->json(['error' => 'User could not be found'], 400);
			}

			//Validate Storage Provider
			$storageProvider = App\StorageProvider::withUuid($request->input('storage_provider_id'))->first();

			if ( !$storageProvider ) {
				return response()->json(['error' => 'Invalid storage provider'], 400);
			}

			//Create or find the file path
			$fileHash = hex2bin( $request->input('hash') );
			$fpHashRaw = sha1( $request->input('file_path'), true );
			$filePath = App\FilePath::where(['hash' => $fpHashRaw, 'user_id' => $user->user_id])->first();

			//Create FilePath if it doesn't exist
			if ( !$filePath ) {

				//Duplicate check should only occur if this is a new file_path
				$preventDuplicate = App\Setting::where(['setting_name' => 'prevent_duplicate_hash', 'value' => 'true'])->first();
				if ( $preventDuplicate ) {
					$duplicate = App\File::where(['hash' => $fileHash, 'user_id' => $user->user_id, 'provider_id' => $storageProvider->provider_id])->first();
					if ( $duplicate ) {
						return response()->json(['error' => 'Duplicate file: File has already been uploaded'], 400);
					}
				}

				$filePath = new App\FilePath;
				$filePath->user_id = $user->user_id;
				$filePath->file_path = $request->input('file_path');
				$filePath->hash = $fpHashRaw;
				$filePath->save();
			}

			//Create or get existing file
			$file = App\File::firstOrNew(['file_path_id' => $filePath->path_id, 'user_id' => $user->user_id, 'file_name' => $request->input('file_name')]);

			if ( !$file->exists ) {
				$file->user_id = $user->user_id;
				$file->client_id = $client->client_id;
				$file->file_name = $request->input('file_name');
				$file->file_path_id = $filePath->path_id;
				$file->file_type = $request->input('file_type');
				$file->file_size = $request->input('file_size');
				$file->hash = $fileHash;
				$file->provider_id = $storageProvider->provider_id;
				$file->save();
			}

			//Add a new file upload
			$upload = new App\FileUpload;
			$upload->file_id = $file->file_id;
			$upload->user_id = $user->user_id;
			$upload->client_id = $client->client_id;
			$upload->parts = 0;
			$upload->total_bytes = $file->file_size;
			$upload->hash = $fileHash;
			$upload->save();

			return response()->json([
				'file' => $file,
				'upload' => $upload
			]);

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
        //Get Upload from DB
				$fileUpload = App\FileUpload::withUuid($id)->firstOrFail();
				$fileUpload->uploaded = $request->input('uploaded');
				$fileUpload->save();

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
