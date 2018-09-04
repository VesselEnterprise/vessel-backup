<?php

namespace App\Http\Controllers\api;

use App;
use Validator;
use Illuminate\Http\Request;
use App\Http\Controllers\Controller;
use Carbon\Carbon;

class AppClientController extends Controller
{

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

		public function install(Request $request)
		{

			//Validate deployment key
			$deployment_key = $request->input('deployment_key');

			if ( empty($deployment_key) ) {
				return response()->json(['error' => 'Invalid deployment key'], 400);
			}

			//return Carbon::now()->toDateTimeString();

			$deployment = App\Deployment::where('deployment_key', $deployment_key)->whereDate('expires_at', '>=', now() )->firstOrFail();

			//Find associated user
			$userName = $request->input('user_name');

			$user = App\User::where(['user_name' => $userName, 'active' => 1])->first();

			if ( !$user ) {
				return response()->json(['error' => 'Invalid user name was provided. Please verify the user exists in the database'], 400);
			}

			//Validate Request
			$validator = Validator::make($request->all(), [
				'client_name' => 'required|string',
				'client_version' => 'required|string'
			]);

			if ( $validator->fails() )
			{
				return response()->json($validator->messages(), 400);
			}

			//Find existing or create new client
			$appClient = App\AppClient::firstOrNew(['client_name' => $request->input('client_name')]);
			$appClient->client_name = $request->input('client_name');
			$appClient->dns_name = $request->input('client_name');
			$appClient->client_version = $request->input('client_version');
			$appClient->ip_address = $request->input('ip_address');
			//$appClient->mac_address = $request->input('mac_address');
			$appClient->os = $request->input('os');
			$appClient->domain = $request->input('domain');
			$appClient->last_check_in = Carbon::now();
			if ( !$appClient->exists || empty($appClient->token) ) {
				$appClient->token = App\AppClient::generateToken();
			}
			$appClient->save();

			//Get storage providers
			$providers = App\StorageProvider::all();
			$appSettings = App\AppSetting::all();

			return response()->json([
				'app_client' => $appClient,
				'storage_providers' => $providers,
				'app_settings' => $appSettings,
				'user' => $user
			]);

		}

}
