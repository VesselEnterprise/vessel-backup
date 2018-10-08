<?php

namespace App\Http\Controllers\api;

use App;
use Illuminate\Http\Request;
use App\Http\Controllers\Controller;

class AzureUploadController extends UploadController
{

		public function initUpload(Request $request) {
			return parent::initUpload($request);
		}

		public function getSignature(Request $request)
		{

			$providerId = $request->input('providerId');
			$stringToSign = base64_decode($request->input('stringToSign'));

			if ( empty($providerId) ) {
				return response()->json(['error' => 'Invalid provider id'], 400);
			}

			if ( !$stringToSign ) {
				return response()->json(['error' => 'Invalid StringToSign'], 400);
			}

			$storageProvider = App\StorageProvider::withUuid($providerId)->firstOrFail();
			$sharedKey = base64_decode($storageProvider->access_key);

			$signature = base64_encode(hash_hmac("sha256", $stringToSign, $sharedKey, true));

			return response()->json(['signature' => $signature]);

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
