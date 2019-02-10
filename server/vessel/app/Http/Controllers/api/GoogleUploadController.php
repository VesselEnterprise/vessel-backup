<?php

namespace App\Http\Controllers\api;

use App;
use Illuminate\Http\Request;
use App\Http\Controllers\Controller;
use Google\Cloud\Storage\StorageClient;
use Carbon\Carbon;

class GoogleUploadController extends UploadController
{
	public function initUpload(Request $request) {
		return parent::initUpload($request);
	}

	public function getSignature(Request $request)
	{

		$providerId = $request->input('providerId'); //Storage Provider
		$objectName = $request->input('objectName');
		$contentMd5 = $request->input('contentMd5');
		$contentType = $request->input('contentType');
		$vesselId = $request->input('vesselId'); //x-goog-vessel-id

		if ( empty($providerId) ) {
			return response()->json(['error' => 'Storage provider must be provided'], 400);
		}

		if ( empty($objectName) ) {
			return response()->json(['error' => 'Object name must be provided'], 400);
		}

		if ( empty($contentMd5) ) {
			return response()->json(['error' => 'MD5 hash must be provided'], 400);
		}

		//Get the Storage Provider config
		$storageProvider = App\StorageProvider::withUuid($providerId)->firstOrFail();

		$objectParams = [];
			'method' => 'PUT',
      'contentMd5' => $contentMd5
		];

		if ( !empty($contentType) ) {
			$objectParams['contentType'] = $contentType;
		}

		if ( !empty($vesselId) ) {
			$objectParams['headers'] = ['x-goog-vessel-id' => $vesselId];
		}

		$storageClient = new StorageClient([
			'keyFilePath' => $storageProvider->key_file
			//'keyFile' => <json contents>
		]);

		//URLs are valid for 7 days
		$ts = Carbon::now()->addDays(7)->getTimestamp();

		$bucket = $storageClient->bucket($storageProvider->bucket_name);
		$object = $bucket->object($objectName);

		$signedUrl = $object->signedUrl($ts, $objectParams);

		echo $signedUrl;

	}

}
