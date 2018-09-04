<?php

namespace App\Http\Controllers\api;

use App;
use Illuminate\Http\Request;
use App\Http\Controllers\Controller;

class AwsUploadController extends UploadController
{

		public function initUpload(Request $request) {
			return parent::initUpload($request);
		}

		public function getSigningKey(Request $request) {

			$providerId = $request->input('providerId');
			$amzDate = $request->input('amzDate');

			if ( empty($amzDate) ) {
				return response()->json(['error' => 'Invalid AWS date string'], 400);
			}

			if ( empty($providerId) ) {
				return response()->json(['error' => 'Invalid provider id'], 400);
			}

			$storageProvider = App\StorageProvider::withUuid($providerId)->firstOrFail();

			$secret = $storageProvider->access_key; //Decrypted from DB automatically using application key
	    $keyDate = hash_hmac("sha256", $amzDate, ("AWS4" . $secret), true);
	    $keyRegion = hash_hmac("sha256", $storageProvider->region, $keyDate, true);
	    $keyService = hash_hmac("sha256", "s3", $keyRegion, true);
	    $keySigning = base64_encode(hash_hmac("sha256", "aws4_request", $keyService, true));

			return response()->json(['key' => $keySigning]);

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
