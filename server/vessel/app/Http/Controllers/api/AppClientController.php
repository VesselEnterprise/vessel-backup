<?php

namespace App\Http\Controllers\api;

use App;
use Validator;
use Laravel\Horizon\Tags;
use Illuminate\Http\Request;
use App\Http\Controllers\Controller;
use Carbon\Carbon;
use Illuminate\Support\Facades\DB;
use Illuminate\Support\Facades\Storage;

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

			$deployment = App\Deployment::where([
				['deployment_key', '=', $deployment_key],
				['expires_at', '>=', Carbon::now()->toDateTimeString()]
			])->orWhere([
				['deployment_key', '=', $deployment_key],
				['never_expires', '=', 1]
			])->firstOrFail();

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
			$appClient->ip_address = $request->ip();
			//$appClient->mac_address = $request->input('mac_address');
			$appClient->os = $request->input('os');
			$appClient->domain = $request->input('domain');
			$appClient->last_check_in = Carbon::now();
			if ( !$appClient->exists || empty($appClient->token) ) {
				$appClient->token = App\AppClient::generateToken();
			}
			$appClient->save();

			//Associate User with client
			$clientUser = App\AppClientUser::firstOrNew(
				['client_id' => $appClient->client_id],
				['user_id' => $user->user_id]
			);
			if ( !$clientUser->exists ) {
				$clientUser->client_id = $appClient->client_id;
				$clientUser->user_id = $user->user_id;
				$clientUser->save();
			}

			//Get storage providers
			$providers = App\StorageProvider::where('active', true)->get();
			$appSettings = App\AppSetting::all();

			//Implement User Settings
			$userSettings = $user->settings;
			foreach($userSettings as $key => $val) {
				$appSettings[$key] = $val;
			}

			return response()->json([
				'app_client' => $appClient,
				'storage_providers' => $providers,
				'app_settings' => $appSettings,
				'user' => $user
			]);

		}

		public function heartbeat(Request $request)
		{

			$client = App\AppClient::where('token', $request->bearerToken() )->firstOrFail();

			if ( !$client ) {
				return response()->json(['error' => 'Bad client'], 400);
			}

			$user = App\User::withUuid($request->input('user_id'))->firstOrFail();

			//Update Client Information
			$client->client_name = $request->input('host_name');
			$client->os = $request->input('os');
			$client->client_version = $request->input('client_version');
			$client->domain = $request->input('domain');
			$client->last_check_in = Carbon::now();
			$client->save();

			//Parse Stats
			$clientStats = $request->input('stats');

			foreach($clientStats as $key => $value) {
					DB::table('app_client_stat')->insert([
						'client_id' => $client->client_id,
						'user_id' => $user->user_id,
						'stat_name' => $key,
						'value' => $value
					]);
			}

			//Process Client Tags
			$clientLogs = $request->input('logs');

			foreach($clientLogs as $key => $value) {
				$logEntry = new App\AppLogEntry();
				$logEntry->client_id = $client->client_id;
				$logEntry->user_id = $user->user_id;
				$logEntry->message = $clientLogs[$key]['message'];
				$logEntry->exception = $clientLogs[$key]['exception'];
				$logEntry->payload = $clientLogs[$key]['payload'];
				$logEntry->code = $clientLogs[$key]['code'];
				$logEntry->error = $clientLogs[$key]['error'];
				$logEntry->type = $clientLogs[$key]['type'];
				$logEntry->logged_at = Carbon::createFromTimestampUTC($clientLogs[$key]['logged_at']);
				$logEntry->save();
			}

			//Return Current App Settings and Storage Providers
			$appSettings = App\AppSetting::all();
			$userSettings = $user->settings;
			foreach($userSettings as $key => $val) {
				$appSettings[$key] = $val;
			}

			$providers = App\StorageProvider::where('active', true)->get();

			return response()->json([
				'message' => '♥',
				'app_settings' => $appSettings,
				'storage_providers' => $providers
			]);

		}

}
