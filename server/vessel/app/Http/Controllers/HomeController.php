<?php

namespace App\Http\Controllers;

use App;
use Illuminate\Http\Request;

class HomeController extends Controller
{
    /**
     * Create a new controller instance.
     *
     * @return void
     */
    public function __construct()
    {
        $this->middleware('auth');
    }

    /**
     * Show the application dashboard.
     *
     * @return \Illuminate\Http\Response
     */
    public function index(Request $request)
    {
				$request->user()->authorizeRoles(['user', 'admin']);

				//Retrieve stats to display on dashboard

				$heartbeats = App\AppClient::orderBy('last_check_in', 'desc')->limit(10)->get();
				$recentBackups = App\File::orderBy('last_backup', 'desc')->limit(10)->get();
        $recentErrors = App\AppLogEntry::where('error', true)->orderBy('logged_at', 'desc')->limit(10)->get();
        $storageProviders = App\StorageProvider::orderBy('provider_name')->get();

        return view('home', [
					'heartbeats' => $heartbeats,
					'recentBackups' => $recentBackups,
          'recentErrors' => $recentErrors,
          'storageProviders' => $storageProviders
				]);
    }
}
